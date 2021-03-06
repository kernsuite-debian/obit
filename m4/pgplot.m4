# autoconf script for pgplot library
AC_DEFUN([AM_PATH_PGPLOT], [
	AC_REQUIRE([AC_F77_LIBRARY_LDFLAGS])dnl
	AC_REQUIRE([AC_PATH_XTRA])dnl

	ac_have_pgplot=no
	PGPLOT_CPPFLAGS=""
	PGPLOT_CFLAGS=""
	PGPLOT_LIBS=""
	PGPLOT_LDFLAGS=""
	AC_SUBST(PGPLOT_CPPFLAGS)
	AC_SUBST(PGPLOT_CFLAGS)
	AC_SUBST(PGPLOT_LDFLAGS)
	AC_SUBST(PGPLOT_LIBS)
])
