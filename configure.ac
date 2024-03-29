dnl Init.
AC_INIT(permrec-excerpt,0.0.1,[],[],[])
AC_PREREQ(2.60)
AC_CONFIG_SRCDIR(README)

dnl Utility function ============================

# AC_CHECK_CPPFLAGS(ADDITIONAL-CPPFLAGS, ACTION-IF-FOUND, ACTION-IF-NOT-FOUND)
#
# checks whether the $(C) compiler accepts the ADDITIONAL-CPPFLAGS
# if so, they are added to the CPPFLAGS
AC_DEFUN([AC_CHECK_CPPFLAGS],
[
  AC_MSG_CHECKING([whether $CPP accepts "$1"])
  temp_check_cppflags="${CPPFLAGS}"
  CPPFLAGS="$1 ${CPPFLAGS}"
  AC_PREPROC_IFELSE(
        [AC_LANG_SOURCE([[int main(void){return 0;}]])],
        [AC_MSG_RESULT([yes])],
        [AC_MSG_RESULT([no]); CPPFLAGS="${temp_check_cppflags}"])
])# AC_CHECK_CPPFLAGS

dnl Utility function ============================

# AC_CHECK_CXXFLAGS(ADDITIONAL-CXXFLAGS, ACTION-IF-FOUND, ACTION-IF-NOT-FOUND)
#
# checks whether the $(CXX) (c++) compiler accepts the ADDITIONAL-CXXFLAGS
# if so, they are added to the CXXFLAGS
AC_DEFUN([AC_CHECK_CXXFLAGS],
[
  AC_MSG_CHECKING([whether $CXX accepts "$1"])
cat > conftest.c++ << EOF
int main(){
  return 0;
}
EOF
if $CXX $CPPFLAGS $CXXFLAGS -o conftest.o conftest.c++ [$1] > /dev/null 2>&1
then
  AC_MSG_RESULT([yes])
  CXXFLAGS="${CXXFLAGS} [$1]"
  AC_CHECK_CPPFLAGS([$1])
  [$2]
else
  AC_MSG_RESULT([no])
  [$3]
fi
])# AC_CHECK_CXXFLAGS

dnl End Utility function ============================

dnl Detect the canonical host and target build environment
AC_CANONICAL_HOST
AC_CANONICAL_BUILD

dnl Setup for automake
AM_INIT_AUTOMAKE([foreign subdir-objects])
AC_CONFIG_HEADER(config.h)

dnl Checks for programs.
AC_PROG_MAKE_SET
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_OBJCXX
AC_PROG_INSTALL
AC_PROG_RANLIB
AC_PREFIX_DEFAULT([/usr/local])

dnl this code needs large file support on 32-bit systems
AC_SYS_LARGEFILE

#Check for big endian machine, should #define WORDS_BIGENDIAN if so
AC_C_BIGENDIAN

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_STRUCT_TM
AC_CHECK_SIZEOF(unsigned char)
AC_CHECK_SIZEOF(unsigned short)
AC_CHECK_SIZEOF(unsigned int)
AC_CHECK_SIZEOF(unsigned long)
AC_CHECK_SIZEOF(unsigned long long)
AC_CHECK_SIZEOF(int *)

dnl some semi complex check for sys/socket so it works on darwin as well
AC_CHECK_HEADERS([stdlib.h sys/types.h])
AC_CHECK_HEADERS([sys/socket.h  netinet/in.h pwd.h], [], [],
[#include <stdio.h>
#ifdef STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# ifdef HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
])

dnl check for the socklen_t (darwin doesn't always have it)
AC_COMPILE_IFELSE([AC_LANG_SOURCE([
#include <stdio.h>
#ifdef STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# ifdef HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
])],[],[AC_DEFINE([socklen_t],[int],[Define to `int` if you don't have socklen_t])])

dnl C++0x is now considered deprecated.
dnl This warning will stay here for a long enough period of time before making C++11 mandatory.

dnl AC_CHECK_CXXFLAGS will prepend gnu++11 to CXXFLAGS if supported, for us.
dnl our code adds -std=gnu++0x if not supported where the function would not do anything.
dnl This code checks the CXX flags but does not keep the changes because AC_CHECK_CXXFLAGS
dnl also adds it to CPPFLAGS which then also affects the C compilation (with warnings like
dnl "-std=gnu++11 is not valid for C")
OLDCFLAGS="$CFLAGS"
OLDCPPFLAGS="$CPPFLAGS"
OLDCXXFLAGS="$CXXFLAGS"

AC_MSG_CHECKING([checking for C++11 support])
AC_CHECK_CXXFLAGS([ -std=gnu++11 ], [  ], [
    warn_cpp0x=1
])

CXXFLAGS="$OLDCXXFLAGS"
CPPFLAGS="$OLDCPPFLAGS"
CFLAGS="$OLDCFLAGS"

if test x$warn_cpp0x = x1; then
    AC_MSG_ERROR([ Your compiler must support C++11 or better ])
fi

CFLAGS="$CFLAGS -std=gnu11";
CXXFLAGS="$CXXFLAGS -std=gnu++11";

dnl FIXME: Remove default "-O2" set by some autotools versions. TODO: check availability of sed.
CFLAGS=["`echo $CFLAGS' ' | sed -e 's/-O[^ ]* //g'`"]
CXXFLAGS=["`echo $CXXFLAGS' ' | sed -e 's/-O[^ ]* //g'`"]

# SDL2
SDL2_CFLAGS=`pkg-config sdl2 --cflags`
SDL2_LIBS=`pkg-config sdl2 --libs`
AC_SUBST(SDL2_CFLAGS)
AC_SUBST(SDL2_LIBS)

dnl I would like to know of any concerns given by the C++ compiler.
dnl Clang/LLVM already does this to some degree, let's get GCC to do it too.
AC_CHECK_CXXFLAGS([ -Wall ])
AC_CHECK_CXXFLAGS([ -Wextra ])
AC_CHECK_CXXFLAGS([ -pedantic ])
AC_CHECK_CXXFLAGS([ -Wconversion ])
AC_CHECK_CXXFLAGS([ -Wsign-promo ])
AC_CHECK_CXXFLAGS([ -Wold-style-cast ])
AC_CHECK_CXXFLAGS([ -Wsign-conversion ])
AC_CHECK_CXXFLAGS([ -Wconversion-null ])
AC_CHECK_CXXFLAGS([ -Wnon-virtual-dtor ])
AC_CHECK_CXXFLAGS([ -Wctor-dtor-privacy ])
AC_CHECK_CXXFLAGS([ -Woverloaded-virtual ])

AC_CONFIG_FILES([ 
Makefile
])
AC_OUTPUT

