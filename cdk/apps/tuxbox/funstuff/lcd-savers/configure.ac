AC_INIT(lcd,0.0.1,,lcd)
AM_INIT_AUTOMAKE(lcd,0.0.1)

AC_PROG_CC

AC_ARG_WITH(target,
	[  --with-target=TARGET    target for compilation [[native,cdk]]],
	[TARGET="$withval"],[TARGET="native"]
)

AC_SUBST(TARGET)
AM_CONDITIONAL(TARGET_NATIVE,test "$TARGET" = "native")
AM_CONDITIONAL(TARGET_CDK,test "$TARGET" = "cdk")

if test "$TARGET" = "cdk"; then
	AC_ARG_WITH(driver,
		[  --with-driver=PATH      path for driver sources [[NONE]]],
		[DRIVER="$withval"],[DRIVER=""])

	if test -d "$DRIVER/include"; then
		AC_DEFINE(HAVE_DBOX2_DRIVER,1,[Define to 1 if you have the dbox2 driver sources])
	else
		AC_MSG_ERROR([can't find driver sources])
	fi

	AC_SUBST(DRIVER)

	CPPFLAGS="$CPPFLAGS -I$DRIVER/include"
fi

AC_OUTPUT([
Makefile
])