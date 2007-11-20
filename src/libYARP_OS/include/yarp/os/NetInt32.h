// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
 * Copyright (C) 2006 Paul Fitzpatrick
 * CopyPolicy: Released under the terms of the GNU GPL v2.0.
 *
 */

#ifndef _YARP2_NETINT32_
#define _YARP2_NETINT32_

// Sleazy trick for big endian systems we haven't tested on yet.
// The two files NetInt32.h and NetFloat64.h need to be cleaned up.
// They can be much simplified by deferring to cmake/autoconf.
#ifdef YARP_BIG_ENDIAN
#define YARP2_OSX
#include <yarp/String.h>
#endif

namespace yarp {
    namespace os {

        /**
         * Definition of the NetInt32 type
         */

#ifdef CYGWIN
#ifndef YARP2_CYGWIN
#define YARP2_CYGWIN
#endif
#endif


#ifndef YARP2_CYGWIN

#ifdef WIN32
#ifndef YARP2_WINDOWS
#define YARP2_WINDOWS
#endif
#endif

#ifdef _WIN32
#ifndef YARP2_WINDOWS
#define YARP2_WINDOWS
#endif
#endif

#ifdef WINDOW2
#ifndef YARP2_WINDOWS
#define YARP2_WINDOWS
#endif
#endif

#ifdef __WIN__
#ifndef YARP2_WINDOWS
#define YARP2_WINDOWS
#endif
#endif

#ifdef __WINDOWS__
#ifndef YARP2_WINDOWS
#define YARP2_WINDOWS
#endif
#endif

#ifdef WINDOWS
#ifndef YARP2_WINDOWS
#define YARP2_WINDOWS
#endif
#endif

#endif /* ifndef CYGWIN */


#ifdef __LINUX__
#ifndef YARP2_LINUX
#define YARP2_LINUX
#endif
#endif

#ifdef __linux__
#ifndef YARP2_LINUX
#define YARP2_LINUX
#endif
#endif

#ifdef __darwin__
#ifndef YARP2_OSX
#define YARP2_OSX
#endif
#endif

#ifdef __DARWIN__
#ifndef YARP2_OSX
#define YARP2_OSX
#endif
#endif

#ifdef DARWIN
#ifndef YARP2_OSX
#define YARP2_OSX
#endif
#endif

#ifdef MACOSX
#ifndef YARP2_OSX
#define YARP2_OSX
#endif
#endif

#ifdef __APPLE__
#ifndef YARP2_OSX
#define YARP2_OSX
#endif
#endif

#ifdef YARP2_LINUX
        typedef int NetInt32;
        //typedef int32_t NetInt32;
#else
#  ifdef YARP2_WINDOWS
#    ifdef __MINGW_H
        typedef int NetInt32;
#    else 
        typedef __int32 NetInt32;
#    endif
#else
#  ifdef YARP2_CYGWIN
        typedef int NetInt32;
#  else
#    ifdef YARP2_OSX


        // in general, this has to come before any reference to sys/types.h
#include <yarp/String.h>
#include <ace/config.h>

        /* OSX begins*/
#ifndef YARP_LITTLE_ENDIAN
#include <sys/types.h>
        typedef int32_t RawNetInt32;
        class NetInt32 {
        private:
            int32_t raw_value;
            int32_t swap(uint32_t x) const {
                return (x>>24) | ((x>>8) & 0xff00) | ((x<<8) & 0xff0000) | (x<<24);
            }
            RawNetInt32 get() const {
                return (int32_t)swap((uint32_t)raw_value);
            }
            void set(RawNetInt32 v) {
                raw_value = (int32_t)swap((uint32_t)v);
            }
        public:
            NetInt32() {
            }
            NetInt32(RawNetInt32 val) {
                set(val);
            }
            operator RawNetInt32() const {
                return get();
            }
            RawNetInt32 operator+(RawNetInt32 v) const {
                return get()+v;
            }
            RawNetInt32 operator-(RawNetInt32 v) const {
                return get()-v;
            }
            RawNetInt32 operator*(RawNetInt32 v) const {
                return get()*v;
            }
            RawNetInt32 operator/(RawNetInt32 v) const {
                return get()/v;
            }
            void operator+=(RawNetInt32 v) {
                set(get()+v);
            }
            void operator-=(RawNetInt32 v) {
                set(get()-v);
            }
            void operator*=(RawNetInt32 v) {
                set(get()*v);
            }
            void operator/=(RawNetInt32 v) {
                set(get()/v);
            }
            void operator++(int) { 
                set(get()+1);
            };
            void operator--(int) { 
                set(get()-1);
            };
        };
#else
        // not really guaranteed to have this
#include <sys/types.h>
        typedef int32_t NetInt32;
#endif
        /* OSX ends*/


#    else
        // now we have to assume we are using autoconf etc
#undef  PACKAGE_BUGREPORT
#undef  PACKAGE_NAME
#undef  PACKAGE_STRING
#undef  PACKAGE_TARNAME
#undef  PACKAGE_VERSION
#    include <yarp/conf/yarp-config.h>
#    if !(WORDS_BIGENDIAN)
#if SIZEOF_INT == 4
        typedef int NetInt32;
#else
#if SIZEOF_SHORT == 4
        typedef short NetInt32;
#else
#      error "could not choose NetInt32 type; see yarp/os/NetInt32.h"
#endif
#endif
#      else
#        error "need to define NetInt32 for this OS; see yarp/os/NetInt32.h"
#      endif
#    endif
#  endif
#endif
#endif

    }
}

#endif /* _YARP2_NETINT32_ */



