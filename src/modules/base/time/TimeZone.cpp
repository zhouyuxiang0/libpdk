// @copyright 2017-2018 zzu_softboy <zzu_softboy@163.com>
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Created by softboy on 2018/01/30.

#include "pdk/base/time/TimeZone.h"
#include "pdk/base/time/internal/TimeZonePrivate.h"
#include "pdk/base/time/DateTime.h"
#include "pdk/base/io/DataStream.h"
#include "pdk/base/lang/String.h"
#include "pdk/utils/SharedData.h"
#ifndef PDK_NO_DEBUG_STREAM
#include "pdk/base/io/Debug.h"
#endif
#include "pdk/global/GlobalStatic.h"
#include <algorithm>

namespace pdk {
namespace time {

using internal::TimeZonePrivate;
using internal::UtcTimeZonePrivate;
using pdk::utils::SharedDataPointer;
using pdk::utils::Locale;
#ifndef PDK_NO_DEBUG_STREAM
using pdk::io::Debug;
using pdk::io::DebugStateSaver;
#endif
using pdk::lang::Latin1String;
using pdk::lang::String;

#if PDK_CONFIG(icu)
using internal::IcuTimeZonePrivate;
#endif
using internal::MacTimeZonePrivate;
#if defined(PDK_OS_UNIX) && !defined(PDK_OS_DARWIN)
using internal::TzTimeZonePrivate;
#endif
#if defined PDK_OS_WIN
using internal::WinTimeZonePrivate;
#endif

namespace {

// Create default time zone using appropriate backend
TimeZonePrivate *new_backend_timezone()
{
#ifdef PDK_NO_SYSTEMLOCALE
#if PDK_CONFIG(icu)
   return new IcuTimeZonePrivate();
#else
   return new UtcTimeZonePrivate();
#endif
#else
#if defined PDK_OS_MAC
   return new MacTimeZonePrivate();
#elif defined(PDK_OS_UNIX)
   return new TzTimeZonePrivate();
   // Registry based timezone backend not available on WinRT
#elif defined PDK_OS_WIN
   return new WinTimeZonePrivate();
#elif PDK_CONFIG(icu)
   return new internal::IcuTimeZonePrivate();
#else
   return new internal::UtcTimeZonePrivate();
#endif // System Locales
#endif // PDK_NO_SYSTEMLOCALE
}

// Create named time zone using appropriate backend
TimeZonePrivate *new_backend_timezone(const ByteArray &ianaId)
{
#ifdef PDK_NO_SYSTEMLOCALE
#if PDK_CONFIG(icu)
   return new IcuTimeZonePrivate(ianaId);
#else
   return new UtcTimeZonePrivate(ianaId);
#endif
#else
#if defined PDK_OS_MAC
   return new MacTimeZonePrivate(ianaId);
#elif defined(PDK_OS_UNIX)
   return new TzTimeZonePrivate(ianaId);
   // Registry based timezone backend not available on WinRT
#elif defined PDK_OS_WIN
   return new WinTimeZonePrivate(ianaId);
#elif PDK_CONFIG(icu)
   return new IcuTimeZonePrivate(ianaId);
#else
   return new UtcTimeZonePrivate(ianaId);
#endif // System Locales
#endif // PDK_NO_SYSTEMLOCALE
}

} // anonymous namespace

class TimeZoneSingleton
{
public:
   TimeZoneSingleton()
      : m_backend(new_backend_timezone())
   {}
   
   // The backend_tz is the tz to use in static methods such as getAvailableTimeZoneIds() and
   // isTimeZoneIdAvailable() and to create named IANA time zones.  This is usually the host
   // system, but may be different if the host resources are insufficient or if
   // PDK_NO_SYSTEMLOCALE is set.  A simple UTC backend is used if no alternative is available.
   SharedDataPointer<TimeZonePrivate> m_backend;
};

PDK_GLOBAL_STATIC(TimeZoneSingleton, sg_globalTimeZone);

TimeZone::TimeZone() noexcept
   : m_implPtr(0)
{}

TimeZone::TimeZone(const ByteArray &ianaId)
{
   // Try and see if it's a valid UTC offset ID, just as quick to try create as look-up
   m_implPtr = new UtcTimeZonePrivate(ianaId);
   // If not a valid UTC offset ID then try create it with the system backend
   // Relies on backend not creating valid tz with invalid name
   if (!m_implPtr->isValid()) {
      m_implPtr = new_backend_timezone(ianaId);
   }
}

TimeZone::TimeZone(int offsetSeconds)
   : m_implPtr((offsetSeconds >= MinUtcOffsetSecs && offsetSeconds <= MaxUtcOffsetSecs)
             ? new UtcTimeZonePrivate(offsetSeconds) : nullptr)
{}

TimeZone::TimeZone(const ByteArray &ianaId, int offsetSeconds, const String &name,
                   const String &abbreviation, Locale::Country country, const String &comment)
   : m_implPtr()
{
   if (!isTimeZoneIdAvailable(ianaId)) {
      m_implPtr = new UtcTimeZonePrivate(ianaId, offsetSeconds, name, abbreviation, country, comment);
   }
}

TimeZone::TimeZone(TimeZonePrivate &dd)
   : m_implPtr(&dd)
{}

TimeZone::TimeZone(const TimeZone &other)
   : m_implPtr(other.m_implPtr)
{}

TimeZone::~TimeZone()
{}

TimeZone &TimeZone::operator=(const TimeZone &other)
{
   m_implPtr = other.m_implPtr;
   return *this;
}

bool TimeZone::operator==(const TimeZone &other) const
{
   if (m_implPtr && other.m_implPtr) {
      return (*m_implPtr == *other.m_implPtr);
   } else {
      return (m_implPtr == other.m_implPtr);
   }
}

bool TimeZone::operator!=(const TimeZone &other) const
{
   if (m_implPtr && other.m_implPtr) {
      return (*m_implPtr != *other.m_implPtr);
   } else {
      return (m_implPtr != other.m_implPtr);
   }
}

bool TimeZone::isValid() const
{
   if (m_implPtr) {
      return m_implPtr->isValid();
   } else {
      return false;
   }
}

ByteArray TimeZone::getId() const
{
   if (m_implPtr) {
      return m_implPtr->getId();
   } else {
      return ByteArray();
   } 
}

Locale::Country TimeZone::getCountry() const
{
   if (isValid()) {
      return m_implPtr->getCountry();
   } else {
      return Locale::Country::AnyCountry;
   }
}

String TimeZone::getComment() const
{
   if (isValid()) {
      return m_implPtr->getComment();
   } else {
      return String();
   }
}

String TimeZone::displayName(const DateTime &atDateTime, NameType nameType,
                             const Locale &locale) const
{
   if (isValid()) {
      return m_implPtr->displayName(atDateTime.toMSecsSinceEpoch(), nameType, locale);
   } else {
      return String();
   }
}

String TimeZone::displayName(TimeType timeType, NameType nameType,
                             const Locale &locale) const
{
   if (isValid()) {
      return m_implPtr->displayName(timeType, nameType, locale);
   } else {
      return String();
   }
}

String TimeZone::abbreviation(const DateTime &atDateTime) const
{
   if (isValid()) {
      return m_implPtr->abbreviation(atDateTime.toMSecsSinceEpoch());
   } else {
      return String();
   }
}

int TimeZone::offsetFromUtc(const DateTime &atDateTime) const
{
   if (isValid()) {
      return m_implPtr->offsetFromUtc(atDateTime.toMSecsSinceEpoch());
   } else {
      return 0;
   }
}

int TimeZone::standardTimeOffset(const DateTime &atDateTime) const
{
   if (isValid()) {
      return m_implPtr->standardTimeOffset(atDateTime.toMSecsSinceEpoch());
   } else {
      return 0;
   }
}

int TimeZone::daylightTimeOffset(const DateTime &atDateTime) const
{
   if (hasDaylightTime()) {
      return m_implPtr->daylightTimeOffset(atDateTime.toMSecsSinceEpoch());
   } else {
      return 0;
   } 
}

bool TimeZone::hasDaylightTime() const
{
   if (isValid()) {
      return m_implPtr->hasDaylightTime();
   } else {
      return false;
   }
}

bool TimeZone::isDaylightTime(const DateTime &atDateTime) const
{
   if (hasDaylightTime()) {
      return m_implPtr->isDaylightTime(atDateTime.toMSecsSinceEpoch());
   } else {
      return false;
   }
}

TimeZone::OffsetData TimeZone::offsetData(const DateTime &forDateTime) const
{
   if (hasTransitions()) {
      return TimeZonePrivate::toOffsetData(m_implPtr->data(forDateTime.toMSecsSinceEpoch()));
   } else {
      return TimeZonePrivate::getInvalidOffsetData();
   }
}

bool TimeZone::hasTransitions() const
{
   if (isValid()) {
      return m_implPtr->hasTransitions();
   } else {
      return false;
   }
}

TimeZone::OffsetData TimeZone::nextTransition(const DateTime &afterDateTime) const
{
   if (hasTransitions()) {
      return TimeZonePrivate::toOffsetData(m_implPtr->nextTransition(afterDateTime.toMSecsSinceEpoch()));
   } else {
      return TimeZonePrivate::getInvalidOffsetData();
   }
}

TimeZone::OffsetData TimeZone::previousTransition(const DateTime &beforeDateTime) const
{
   if (hasTransitions()) {
      return TimeZonePrivate::toOffsetData(m_implPtr->previousTransition(beforeDateTime.toMSecsSinceEpoch()));
   } else {
      return TimeZonePrivate::getInvalidOffsetData();
   }
}

TimeZone::OffsetDataList TimeZone::transitions(const DateTime &fromDateTime,
                                               const DateTime &toDateTime) const
{
   OffsetDataList list;
   if (hasTransitions()) {
      const TimeZonePrivate::DataList plist = m_implPtr->transitions(fromDateTime.toMSecsSinceEpoch(),
                                                                   toDateTime.toMSecsSinceEpoch());
      list.reserve(plist.size());
      for (const TimeZonePrivate::Data &pdata : plist) {
         list.push_back(TimeZonePrivate::toOffsetData(pdata));
      }
   }
   return list;
}

// Static methods

ByteArray TimeZone::getSystemTimeZoneId()
{
   return sg_globalTimeZone->m_backend->getSystemTimeZoneId();
}

TimeZone TimeZone::getSystemTimeZone()
{
   return TimeZone(TimeZone::getSystemTimeZoneId());
}

TimeZone TimeZone::getUtc()
{
   return TimeZone(TimeZonePrivate::getUtcByteArray());
}

bool TimeZone::isTimeZoneIdAvailable(const ByteArray &ianaId)
{
   // isValidId is not strictly required, but faster to weed out invalid
   // IDs as availableTimeZoneIds() may be slow
   if (!TimeZonePrivate::isValidId(ianaId)) {
      return false;
   }
   const std::list<ByteArray> tzIds = getAvailableTimeZoneIds();
   return std::binary_search(tzIds.begin(), tzIds.end(), ianaId);
}

namespace {

std::list<ByteArray> set_union(const std::list<ByteArray> &l1, const std::list<ByteArray> &l2)
{
   std::list<ByteArray> result;
   result.resize(l1.size() + l2.size());
   std::set_union(l1.begin(), l1.end(),
                  l2.begin(), l2.end(),
                  std::back_inserter(result));
   return result;
}

} // anonymous namespace

std::list<ByteArray> TimeZone::getAvailableTimeZoneIds()
{
   return set_union(UtcTimeZonePrivate().getAvailableTimeZoneIds(),
                    sg_globalTimeZone->m_backend->getAvailableTimeZoneIds());
}

std::list<ByteArray> TimeZone::getAvailableTimeZoneIds(Locale::Country country)
{
   return set_union(UtcTimeZonePrivate().getAvailableTimeZoneIds(country),
                    sg_globalTimeZone->m_backend->getAvailableTimeZoneIds(country));
}

std::list<ByteArray> TimeZone::getAvailableTimeZoneIds(int offsetSeconds)
{
   return set_union(UtcTimeZonePrivate().getAvailableTimeZoneIds(offsetSeconds),
                    sg_globalTimeZone->m_backend->getAvailableTimeZoneIds(offsetSeconds));
}

ByteArray TimeZone::ianaIdToWindowsId(const ByteArray &ianaId)
{
   return TimeZonePrivate::ianaIdToWindowsId(ianaId);
}

ByteArray TimeZone::windowsIdToDefaultIanaId(const ByteArray &windowsId)
{
   return TimeZonePrivate::windowsIdToDefaultIanaId(windowsId);
}

ByteArray TimeZone::windowsIdToDefaultIanaId(const ByteArray &windowsId,
                                             Locale::Country country)
{
   return TimeZonePrivate::windowsIdToDefaultIanaId(windowsId, country);
}

std::list<ByteArray> TimeZone::windowsIdToIanaIds(const ByteArray &windowsId)
{
   return TimeZonePrivate::windowsIdToIanaIds(windowsId);
}

std::list<ByteArray> TimeZone::windowsIdToIanaIds(const ByteArray &windowsId,
                                                  Locale::Country country)
{
   return TimeZonePrivate::windowsIdToIanaIds(windowsId, country);
}

#ifndef PDK_NO_DATASTREAM
DataStream &operator<<(DataStream &ds, const TimeZone &tz)
{
   tz.m_implPtr->serialize(ds);
   return ds;
}

DataStream &operator>>(DataStream &ds, TimeZone &tz)
{
   String ianaId;
   ds >> ianaId;
   if (ianaId == Latin1String("OffsetFromUtc")) {
      int utcOffset;
      String name;
      String abbreviation;
      int country;
      String comment;
      ds >> ianaId >> utcOffset >> name >> abbreviation >> country >> comment;
      // Try creating as a system timezone, which succeeds (producing a valid
      // zone) iff ianaId is valid; we can then ignore the other data.
      tz = TimeZone(ianaId.toUtf8());
      // If not, then construct a custom timezone using all the saved values:
      if (!tz.isValid())
         tz = TimeZone(ianaId.toUtf8(), utcOffset, name, abbreviation,
                       Locale::Country(country), comment);
   } else {
      tz = TimeZone(ianaId.toUtf8());
   }
   return ds;
}
#endif // PDK_NO_DATASTREAM

#ifndef PDK_NO_DEBUG_STREAM
Debug operator<<(Debug dbg, const TimeZone &tz)
{
   DebugStateSaver saver(dbg);
   //TODO Include backend and data version details?
   dbg.nospace() << "TimeZone(" << String::fromUtf8(tz.getId()) << ')';
   return dbg;
}
#endif

} // time
} // pdk
