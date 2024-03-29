
/********************************************************************
 * COPYRIGHT: 
 * Copyright (c) 1997-2013, International Business Machines Corporation and
 * others. All Rights Reserved.
 ********************************************************************/

#include "unicode/utypes.h"

#if !UCONFIG_NO_FORMATTING


//TODO: define it in compiler flag
//#define DTIFMTTS_DEBUG 1


#ifdef DTIFMTTS_DEBUG 
#include <iostream>
#endif


#include "cstring.h"
#include "dtifmtts.h"
#include "unicode/gregocal.h"
#include "unicode/dtintrv.h"
#include "unicode/dtitvinf.h"
#include "unicode/dtitvfmt.h"
#include "unicode/localpointer.h"
#include "unicode/timezone.h"



#ifdef DTIFMTTS_DEBUG 
//#define PRINTMESG(msg) { std::cout << "(" << __FILE__ << ":" << __LINE__ << ") " << msg << "\n"; }
#define PRINTMESG(msg) { std::cout << msg; }
#endif

#define ARRAY_SIZE(array) (sizeof array / sizeof array[0])

#include <stdio.h>


void DateIntervalFormatTest::runIndexedTest( int32_t index, UBool exec, const char* &name, char* /*par*/ ) {
    if (exec) logln("TestSuite DateIntervalFormat");
    switch (index) {
        TESTCASE(0, testAPI);
        TESTCASE(1, testFormat);
        TESTCASE(2, testFormatUserDII);
        TESTCASE(3, testSetIntervalPatternNoSideEffect);
        TESTCASE(4, testYearFormats);
        TESTCASE(5, testStress);
        default: name = ""; break;
    }
}

/**
 * Test various generic API methods of DateIntervalFormat for API coverage.
 */
void DateIntervalFormatTest::testAPI() {

    /* ====== Test create interval instance with default locale and skeleton
     */
    UErrorCode status = U_ZERO_ERROR;
    logln("Testing DateIntervalFormat create instance with default locale and skeleton");
 
    DateIntervalFormat* dtitvfmt = DateIntervalFormat::createInstance(UDAT_YEAR_MONTH_DAY, status);
    if(U_FAILURE(status)) {
        dataerrln("ERROR: Could not create DateIntervalFormat (skeleton + default locale) - exitting");
        return;
    } else {
        delete dtitvfmt;
    }


    /* ====== Test create interval instance with given locale and skeleton
     */
    status = U_ZERO_ERROR;
    logln("Testing DateIntervalFormat create instance with given locale and skeleton");
 
    dtitvfmt = DateIntervalFormat::createInstance(UDAT_YEAR_MONTH_DAY, Locale::getJapanese(), status);
    if(U_FAILURE(status)) {
        dataerrln("ERROR: Could not create DateIntervalFormat (skeleton + locale) - exitting");
        return;
    } else {
        delete dtitvfmt;
    }


    /* ====== Test create interval instance with dateIntervalInfo and skeleton
     */
    status = U_ZERO_ERROR;
    logln("Testing DateIntervalFormat create instance with dateIntervalInfo  and skeleton");
 
    DateIntervalInfo* dtitvinf = new DateIntervalInfo(Locale::getSimplifiedChinese(), status);

    dtitvfmt = DateIntervalFormat::createInstance("EEEdMMMyhms", *dtitvinf, status);
    delete dtitvinf;

    if(U_FAILURE(status)) {
        dataerrln("ERROR: Could not create DateIntervalFormat (skeleton + DateIntervalInfo + default locale) - exitting");
        return;
    } else {
        delete dtitvfmt;
    } 


    /* ====== Test create interval instance with dateIntervalInfo and skeleton
     */
    status = U_ZERO_ERROR;
    logln("Testing DateIntervalFormat create instance with dateIntervalInfo  and skeleton");
 
    dtitvinf = new DateIntervalInfo(Locale::getSimplifiedChinese(), status);

    dtitvfmt = DateIntervalFormat::createInstance("EEEdMMMyhms", Locale::getSimplifiedChinese(), *dtitvinf, status);
    delete dtitvinf;
    if(U_FAILURE(status)) {
        dataerrln("ERROR: Could not create DateIntervalFormat (skeleton + DateIntervalInfo + locale) - exitting");
        return;
    } 
    // not deleted, test clone 


    // ====== Test clone()
    status = U_ZERO_ERROR;
    logln("Testing DateIntervalFormat clone");

    DateIntervalFormat* another = (DateIntervalFormat*)dtitvfmt->clone();
    if ( (*another) != (*dtitvfmt) ) {
        dataerrln("ERROR: clone failed");
    }


    // ====== Test getDateIntervalInfo, setDateIntervalInfo, adoptDateIntervalInfo
    status = U_ZERO_ERROR;
    logln("Testing DateIntervalFormat getDateIntervalInfo");
    const DateIntervalInfo* inf = another->getDateIntervalInfo();
    dtitvfmt->setDateIntervalInfo(*inf, status);
    const DateIntervalInfo* anotherInf = dtitvfmt->getDateIntervalInfo();
    if ( (*inf) != (*anotherInf) || U_FAILURE(status) ) {
        dataerrln("ERROR: getDateIntervalInfo/setDateIntervalInfo failed");
    }

    {
        // We make sure that setDateIntervalInfo does not corrupt the cache. See ticket 9919.
        status = U_ZERO_ERROR;
        logln("Testing DateIntervalFormat setDateIntervalInfo");
        const Locale &enLocale = Locale::getEnglish();
        LocalPointer<DateIntervalFormat> dif(DateIntervalFormat::createInstance("yMd", enLocale, status));
        if (U_FAILURE(status)) {
            errln("Failure encountered: %s", u_errorName(status));
            return;
        }
        UnicodeString expected;
        LocalPointer<Calendar> fromTime(Calendar::createInstance(enLocale, status));
        LocalPointer<Calendar> toTime(Calendar::createInstance(enLocale, status));
        if (U_FAILURE(status)) {
            errln("Failure encountered: %s", u_errorName(status));
            return;
        }
        FieldPosition pos = 0;
        fromTime->set(2013, 3, 26);
        toTime->set(2013, 3, 28);
        dif->format(*fromTime, *toTime, expected, pos, status);
        if (U_FAILURE(status)) {
            errln("Failure encountered: %s", u_errorName(status));
            return;
        }
        LocalPointer<DateIntervalInfo> dii(new DateIntervalInfo(Locale::getEnglish(), status));
        if (U_FAILURE(status)) {
            errln("Failure encountered: %s", u_errorName(status));
            return;
        }
        dii->setIntervalPattern(ctou("yMd"), UCAL_DATE, ctou("M/d/y \\u2013 d"), status);
        dif->setDateIntervalInfo(*dii, status);
        if (U_FAILURE(status)) {
            errln("Failure encountered: %s", u_errorName(status));
            return;
        }
        dif.adoptInstead(DateIntervalFormat::createInstance("yMd", enLocale, status));
        if (U_FAILURE(status)) {
            errln("Failure encountered: %s", u_errorName(status));
            return;
        }
        UnicodeString actual;
        pos = 0;
        dif->format(*fromTime, *toTime, actual, pos, status);
        if (U_FAILURE(status)) {
            errln("Failure encountered: %s", u_errorName(status));
            return;
        }
        if (expected != actual) {
            errln("DateIntervalFormat.setIntervalInfo should have no side effects.");
        }
    }
    
    /*
    status = U_ZERO_ERROR;
    DateIntervalInfo* nonConstInf = inf->clone();
    dtitvfmt->adoptDateIntervalInfo(nonConstInf, status);
    anotherInf = dtitvfmt->getDateIntervalInfo();
    if ( (*inf) != (*anotherInf) || U_FAILURE(status) ) {
        dataerrln("ERROR: adoptDateIntervalInfo failed");
    }
    */

    // ====== Test getDateFormat, setDateFormat, adoptDateFormat
    
    status = U_ZERO_ERROR;
    logln("Testing DateIntervalFormat getDateFormat");
    /*
    const DateFormat* fmt = another->getDateFormat();
    dtitvfmt->setDateFormat(*fmt, status);
    const DateFormat* anotherFmt = dtitvfmt->getDateFormat();
    if ( (*fmt) != (*anotherFmt) || U_FAILURE(status) ) {
        dataerrln("ERROR: getDateFormat/setDateFormat failed");
    }

    status = U_ZERO_ERROR;
    DateFormat* nonConstFmt = (DateFormat*)fmt->clone();
    dtitvfmt->adoptDateFormat(nonConstFmt, status);
    anotherFmt = dtitvfmt->getDateFormat();
    if ( (*fmt) != (*anotherFmt) || U_FAILURE(status) ) {
        dataerrln("ERROR: adoptDateFormat failed");
    }
    delete fmt;
    */


    // ======= Test getStaticClassID()

    logln("Testing getStaticClassID()");


    if(dtitvfmt->getDynamicClassID() != DateIntervalFormat::getStaticClassID()) {
        errln("ERROR: getDynamicClassID() didn't return the expected value");
    }
    
    delete another;

    // ====== test constructor/copy constructor and assignment
    /* they are protected, no test
    logln("Testing DateIntervalFormat constructor and assigment operator");
    status = U_ZERO_ERROR;

    DateFormat* constFmt = (constFmt*)dtitvfmt->getDateFormat()->clone();
    inf = dtitvfmt->getDateIntervalInfo()->clone();


    DateIntervalFormat* dtifmt = new DateIntervalFormat(fmt, inf, status);
    if(U_FAILURE(status)) {
        dataerrln("ERROR: Could not create DateIntervalFormat (default) - exitting");
        return;
    } 

    DateIntervalFormat* dtifmt2 = new(dtifmt);
    if ( (*dtifmt) != (*dtifmt2) ) {
        dataerrln("ERROR: Could not create DateIntervalFormat (default) - exitting");
        return;
    }

    DateIntervalFormat dtifmt3 = (*dtifmt);
    if ( (*dtifmt) != dtifmt3 ) {
        dataerrln("ERROR: Could not create DateIntervalFormat (default) - exitting");
        return;
    }

    delete dtifmt2;
    delete dtifmt3;
    delete dtifmt;
    */


    //===== test format and parse ==================
    Formattable formattable;
    formattable.setInt64(10);
    UnicodeString res;
    FieldPosition pos = 0;
    status = U_ZERO_ERROR;
    dtitvfmt->format(formattable, res, pos, status);
    if ( status != U_ILLEGAL_ARGUMENT_ERROR ) {
        dataerrln("ERROR: format non-date-interval object should set U_ILLEGAL_ARGUMENT_ERROR - exitting");
        return;
    }

    DateInterval* dtitv = new DateInterval(3600*24*365, 3600*24*366);
    formattable.adoptObject(dtitv);
    res.remove();
    pos = 0;
    status = U_ZERO_ERROR;
    dtitvfmt->format(formattable, res, pos, status);
    if ( U_FAILURE(status) ) {
        dataerrln("ERROR: format date interval failed - exitting");
        return;
    }

    const DateFormat* dfmt = dtitvfmt->getDateFormat();
    Calendar* fromCal = dfmt->getCalendar()->clone();
    Calendar* toCal = dfmt->getCalendar()->clone();
    res.remove();
    pos = 0;
    status = U_ZERO_ERROR;
    dtitvfmt->format(*fromCal, *toCal, res, pos, status);
    if ( U_FAILURE(status) ) {
        dataerrln("ERROR: format date interval failed - exitting");
        return;
    }
    delete fromCal;
    delete toCal;


    Formattable fmttable;
    status = U_ZERO_ERROR;
    // TODO: why do I need cast?
    ((Format*)dtitvfmt)->parseObject(res, fmttable, status);
    if ( status != U_INVALID_FORMAT_ERROR ) {
        dataerrln("ERROR: parse should set U_INVALID_FORMAT_ERROR - exitting");
        return;
    }

    delete dtitvfmt;

    //====== test setting time zone
    logln("Testing DateIntervalFormat set & format with different time zones, get time zone");
    status = U_ZERO_ERROR;
    dtitvfmt = DateIntervalFormat::createInstance("MMMdHHmm", Locale::getEnglish(), status);
    if ( U_SUCCESS(status) ) {
        UDate date1 = 1299090600000.0; // 2011-Mar-02 1030 in US/Pacific, 2011-Mar-03 0330 in Asia/Tokyo
        UDate date2 = 1299115800000.0; // 2011-Mar-02 1730 in US/Pacific, 2011-Mar-03 1030 in Asia/Tokyo
        
        DateInterval * dtitv12 = new DateInterval(date1, date2);
        TimeZone * tzCalif = TimeZone::createTimeZone("US/Pacific");
        TimeZone * tzTokyo = TimeZone::createTimeZone("Asia/Tokyo");
        UnicodeString fmtCalif = UnicodeString(ctou("Mar 2, 10:30 - 17:30"));
        UnicodeString fmtTokyo = UnicodeString(ctou("Mar 3, 03:30 - 10:30"));

        dtitvfmt->adoptTimeZone(tzCalif);
        res.remove();
        pos = 0;
        status = U_ZERO_ERROR;
        dtitvfmt->format(dtitv12, res, pos, status);
        if ( U_SUCCESS(status) ) {
            if ( res.compare(fmtCalif) != 0 ) {
                errln("ERROR: DateIntervalFormat::format for tzCalif, expect " + fmtCalif + ", get " + res);
            }
        } else {
            errln("ERROR: DateIntervalFormat::format for tzCalif, status %s", u_errorName(status));
        }

        dtitvfmt->setTimeZone(*tzTokyo);
        res.remove();
        pos = 0;
        status = U_ZERO_ERROR;
        dtitvfmt->format(dtitv12, res, pos, status);
        if ( U_SUCCESS(status) ) {
            if ( res.compare(fmtTokyo) != 0 ) {
                errln("ERROR: DateIntervalFormat::format for fmtTokyo, expect " + fmtTokyo + ", get " + res);
            }
        } else {
            errln("ERROR: DateIntervalFormat::format for tzTokyo, status %s", u_errorName(status));
        }
        
        if ( dtitvfmt->getTimeZone() != *tzTokyo ) {
            errln("ERROR: DateIntervalFormat::getTimeZone returns mismatch.");
        }

        delete tzTokyo; // tzCalif was owned by dtitvfmt which should have deleted it
        delete dtitv12;
        delete dtitvfmt;
    } else {
        errln("ERROR: DateIntervalFormat::createInstance(\"MdHH\", Locale::getEnglish(), ...), status %s", u_errorName(status));
    }
    //====== test format  in testFormat()
    
    //====== test DateInterval class (better coverage)
    DateInterval dtitv1(3600*24*365, 3600*24*366);
    DateInterval dtitv2(dtitv1);

    if (!(dtitv1 == dtitv2)) {
        errln("ERROR: Copy constructor failed for DateInterval.");
    }

    DateInterval dtitv3(3600*365, 3600*366);
    dtitv3 = dtitv1;
    if (!(dtitv3 == dtitv1)) {
        errln("ERROR: Equal operator failed for DateInterval.");
    }

    DateInterval *dtitv4 = dtitv1.clone();
    if (*dtitv4 != dtitv1) {
        errln("ERROR: Equal operator failed for DateInterval.");
    }
    delete dtitv4;
}


/**
 * Test format
 */
void DateIntervalFormatTest::testFormat() {
    // first item is date pattern
    // followed by a group of locale/from_data/to_data/skeleton/interval_data
    const char* DATA[] = {
        "yyyy MM dd HH:mm:ss",    
        // test root
        "root", "2007 11 10 10:10:10", "2007 12 10 10:10:10", "yM", "2007-11 \\u2013 2007-12",
         
        // test 'H' and 'h', using availableFormat in fallback
        "en", "2007 11 10 10:10:10", "2007 11 10 15:10:10", "Hms", "10:10:10 - 15:10:10",
        "en", "2007 11 10 10:10:10", "2007 11 10 15:10:10", "hms", "10:10:10 AM - 3:10:10 PM",

        "en", "2007 10 10 10:10:10", "2008 10 10 10:10:10", "MMMM", "October 2007 - October 2008", 
        "en", "2007 10 10 10:10:10", "2008 10 10 10:10:10", "MMM", "Oct 2007 - Oct 2008", 
        // test skeleton with both date and time
        "en", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "dMMMyhm", "Nov 10, 2007, 10:10 AM - Nov 20, 2007, 10:10 AM", 
        
        "en", "2007 11 10 10:10:10", "2007 11 10 11:10:10", "dMMMyhm", "Nov 10, 2007, 10:10 - 11:10 AM", 
        
        "en", "2007 11 10 10:10:10", "2007 11 10 11:10:10", "hms", "10:10:10 AM - 11:10:10 AM", 
        "en", "2007 11 10 10:10:10", "2007 11 10 11:10:10", "Hms", "10:10:10 - 11:10:10", 
        "en", "2007 11 10 20:10:10", "2007 11 10 21:10:10", "Hms", "20:10:10 - 21:10:10", 
        
        "en", "2007 10 10 10:10:10", "2008 10 10 10:10:10", "EEEEdMMMMy", "Wednesday, October 10, 2007 - Friday, October 10, 2008", 
        
        "en", "2007 10 10 10:10:10", "2008 10 10 10:10:10", "dMMMMy", "October 10, 2007 - October 10, 2008", 
        
        "en", "2007 10 10 10:10:10", "2008 10 10 10:10:10", "dMMMM", "October 10, 2007 - October 10, 2008", 
        
        "en", "2007 10 10 10:10:10", "2008 10 10 10:10:10", "MMMMy", "October 2007 - October 2008", 
        
        "en", "2007 10 10 10:10:10", "2008 10 10 10:10:10", "EEEEdMMMM", "Wednesday, October 10, 2007 - Friday, October 10, 2008", 
        
        "en", "2007 10 10 10:10:10", "2008 10 10 10:10:10", "EdMMMy", "Wed, Oct 10, 2007 - Fri, Oct 10, 2008", 
        
        "en", "2007 10 10 10:10:10", "2008 10 10 10:10:10", "dMMMy", "Oct 10, 2007 - Oct 10, 2008", 
        
        "en", "2007 10 10 10:10:10", "2008 10 10 10:10:10", "dMMM", "Oct 10, 2007 - Oct 10, 2008", 
        
        "en", "2007 10 10 10:10:10", "2008 10 10 10:10:10", "MMMy", "Oct 2007 - Oct 2008", 
        
        "en", "2007 10 10 10:10:10", "2008 10 10 10:10:10", "EdMMM", "Wed, Oct 10, 2007 - Fri, Oct 10, 2008", 
        
        "en", "2007 10 10 10:10:10", "2008 10 10 10:10:10", "EdMy", "Wed, 10/10/2007 - Fri, 10/10/2008", 
        
        "en", "2007 10 10 10:10:10", "2008 10 10 10:10:10", "dMy", "10/10/2007 - 10/10/2008", 
        
        "en", "2007 10 10 10:10:10", "2008 10 10 10:10:10", "dM", "10/10/2007 - 10/10/2008", 
        
        "en", "2007 10 10 10:10:10", "2008 10 10 10:10:10", "My", "10/2007 - 10/2008", 
        
        "en", "2007 10 10 10:10:10", "2008 10 10 10:10:10", "EdM", "Wed, 10/10/2007 - Fri, 10/10/2008", 
        
        "en", "2007 10 10 10:10:10", "2008 10 10 10:10:10", "d", "10/10/2007 - 10/10/2008", 
        
        "en", "2007 10 10 10:10:10", "2008 10 10 10:10:10", "Ed", "10 Wed - 10 Fri", 
        
        "en", "2007 10 10 10:10:10", "2008 10 10 10:10:10", "y", "2007 - 2008", 
        
        "en", "2007 10 10 10:10:10", "2008 10 10 10:10:10", "M", "10/2007 - 10/2008", 
        
        
        
        "en", "2007 10 10 10:10:10", "2008 10 10 10:10:10", "hm", "10/10/2007, 10:10 AM - 10/10/2008, 10:10 AM", 
        "en", "2007 10 10 10:10:10", "2008 10 10 10:10:10", "Hm", "10/10/2007, 10:10 - 10/10/2008, 10:10", 
        "en", "2007 10 10 20:10:10", "2008 10 10 20:10:10", "Hm", "10/10/2007, 20:10 - 10/10/2008, 20:10", 
        
        "en", "2007 10 10 10:10:10", "2008 10 10 10:10:10", "hmv", "10/10/2007, 10:10 AM PT - 10/10/2008, 10:10 AM PT", 
        
        "en", "2007 10 10 10:10:10", "2008 10 10 10:10:10", "hmz", "10/10/2007, 10:10 AM PDT - 10/10/2008, 10:10 AM PDT", 
        
        "en", "2007 10 10 10:10:10", "2008 10 10 10:10:10", "h", "10/10/2007, 10 AM - 10/10/2008, 10 AM", 
        
        "en", "2007 10 10 10:10:10", "2008 10 10 10:10:10", "hv", "10/10/2007, 10 AM PT - 10/10/2008, 10 AM PT", 
        
        "en", "2007 10 10 10:10:10", "2008 10 10 10:10:10", "hz", "10/10/2007, 10 AM PDT - 10/10/2008, 10 AM PDT", 
        
        "en", "2007 10 10 10:10:10", "2008 10 10 10:10:10", "EEddMMyyyy", "Wed, 10/10/2007 - Fri, 10/10/2008", 
        
        "en", "2007 10 10 10:10:10", "2008 10 10 10:10:10", "EddMMy", "Wed, 10/10/2007 - Fri, 10/10/2008", 
        
        "en", "2007 10 10 10:10:10", "2008 10 10 10:10:10", "hhmm", "10/10/2007, 10:10 AM - 10/10/2008, 10:10 AM", 
        
        "en", "2007 10 10 10:10:10", "2008 10 10 10:10:10", "hhmmzz", "10/10/2007, 10:10 AM PDT - 10/10/2008, 10:10 AM PDT", 
        
        "en", "2007 10 10 10:10:10", "2008 10 10 10:10:10", "hms", "10/10/2007, 10:10:10 AM - 10/10/2008, 10:10:10 AM", 
        
        "en", "2007 10 10 10:10:10", "2008 10 10 10:10:10", "dMMMMMy", "O 10, 2007 - O 10, 2008", 
        
        "en", "2007 10 10 10:10:10", "2008 10 10 10:10:10", "EEEEEdM", "W, 10/10/2007 - F, 10/10/2008", 
        
        "en", "2007 10 10 10:10:10", "2007 11 10 10:10:10", "EEEEdMMMMy", "Wednesday, October 10 - Saturday, November 10, 2007", 
        
        "en", "2007 10 10 10:10:10", "2007 11 10 10:10:10", "dMMMMy", "October 10 - November 10, 2007", 
        
        "en", "2007 10 10 10:10:10", "2007 11 10 10:10:10", "dMMMM", "October 10 - November 10", 
        
        "en", "2007 10 10 10:10:10", "2007 11 10 10:10:10", "MMMMy", "October - November 2007", 
        
        "en", "2007 10 10 10:10:10", "2007 11 10 10:10:10", "EEEEdMMMM", "Wednesday, October 10 - Saturday, November 10", 
        
        "en", "2007 10 10 10:10:10", "2007 11 10 10:10:10", "EdMMMy", "Wed, Oct 10 - Sat, Nov 10, 2007", 
        
        "en", "2007 10 10 10:10:10", "2007 11 10 10:10:10", "dMMMy", "Oct 10 - Nov 10, 2007", 
        
        "en", "2007 10 10 10:10:10", "2007 11 10 10:10:10", "dMMM", "Oct 10 - Nov 10", 
        
        "en", "2007 10 10 10:10:10", "2007 11 10 10:10:10", "MMMy", "Oct - Nov 2007", 
        
        "en", "2007 10 10 10:10:10", "2007 11 10 10:10:10", "EdMMM", "Wed, Oct 10 - Sat, Nov 10", 
        
        "en", "2007 10 10 10:10:10", "2007 11 10 10:10:10", "EdMy", "Wed, 10/10/2007 - Sat, 11/10/2007", 
        
        "en", "2007 10 10 10:10:10", "2007 11 10 10:10:10", "dMy", "10/10/2007 - 11/10/2007", 
        
        
        "en", "2007 10 10 10:10:10", "2007 11 10 10:10:10", "My", "10/2007 - 11/2007", 
        
        "en", "2007 10 10 10:10:10", "2007 11 10 10:10:10", "EdM", "Wed, 10/10 - Sat, 11/10", 
        
        "en", "2007 10 10 10:10:10", "2007 11 10 10:10:10", "d", "10/10 - 11/10", 
        
        "en", "2007 10 10 10:10:10", "2007 11 10 10:10:10", "Ed", "10 Wed - 10 Sat", 
        
        "en", "2007 10 10 10:10:10", "2007 11 10 10:10:10", "y", "2007", 
        
        "en", "2007 10 10 10:10:10", "2007 11 10 10:10:10", "M", "10 - 11", 
        
        "en", "2007 10 10 10:10:10", "2007 11 10 10:10:10", "MMM", "Oct - Nov", 
        
        "en", "2007 10 10 10:10:10", "2007 11 10 10:10:10", "MMMM", "October - November", 
        
        "en", "2007 10 10 10:10:10", "2007 11 10 10:10:10", "hm", "10/10/2007, 10:10 AM - 11/10/2007, 10:10 AM", 
        "en", "2007 10 10 10:10:10", "2007 11 10 10:10:10", "Hm", "10/10/2007, 10:10 - 11/10/2007, 10:10", 
        "en", "2007 10 10 20:10:10", "2007 11 10 20:10:10", "Hm", "10/10/2007, 20:10 - 11/10/2007, 20:10", 
        
        "en", "2007 10 10 10:10:10", "2007 11 10 10:10:10", "hmv", "10/10/2007, 10:10 AM PT - 11/10/2007, 10:10 AM PT", 
        
        "en", "2007 10 10 10:10:10", "2007 11 10 10:10:10", "hmz", "10/10/2007, 10:10 AM PDT - 11/10/2007, 10:10 AM PST", 
        
        "en", "2007 10 10 10:10:10", "2007 11 10 10:10:10", "h", "10/10/2007, 10 AM - 11/10/2007, 10 AM", 
        
        "en", "2007 10 10 10:10:10", "2007 11 10 10:10:10", "hv", "10/10/2007, 10 AM PT - 11/10/2007, 10 AM PT", 
        
        "en", "2007 10 10 10:10:10", "2007 11 10 10:10:10", "hz", "10/10/2007, 10 AM PDT - 11/10/2007, 10 AM PST", 
        
        "en", "2007 10 10 10:10:10", "2007 11 10 10:10:10", "EEddMMyyyy", "Wed, 10/10/2007 - Sat, 11/10/2007", 
        
        "en", "2007 10 10 10:10:10", "2007 11 10 10:10:10", "EddMMy", "Wed, 10/10/2007 - Sat, 11/10/2007", 
        
        
        "en", "2007 10 10 10:10:10", "2007 11 10 10:10:10", "hhmmzz", "10/10/2007, 10:10 AM PDT - 11/10/2007, 10:10 AM PST", 
        
        "en", "2007 10 10 10:10:10", "2007 11 10 10:10:10", "hms", "10/10/2007, 10:10:10 AM - 11/10/2007, 10:10:10 AM", 
        
        "en", "2007 10 10 10:10:10", "2007 11 10 10:10:10", "dMMMMMy", "O 10 - N 10, 2007", 
        
        "en", "2007 10 10 10:10:10", "2007 11 10 10:10:10", "EEEEEdM", "W, 10/10 - S, 11/10", 
        
        "en", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "EEEEdMMMMy", "Saturday, November 10 - Tuesday, November 20, 2007", 
        
        "en", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "dMMMMy", "November 10 - 20, 2007", 
        
        "en", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "dMMMM", "November 10 - 20", 
        
        
        "en", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "EEEEdMMMM", "Saturday, November 10 - Tuesday, November 20", 
        
        "en", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "EdMMMy", "Sat, Nov 10 - Tue, Nov 20, 2007", 
        
        "en", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "dMMMy", "Nov 10 - 20, 2007", 
        
        "en", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "dMMM", "Nov 10 - 20", 
        
        "en", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "MMMy", "Nov 2007", 
        
        "en", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "EdMMM", "Sat, Nov 10 - Tue, Nov 20", 
        
        "en", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "EdMy", "Sat, 11/10/2007 - Tue, 11/20/2007", 
        
        "en", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "dMy", "11/10/2007 - 11/20/2007", 
        
        "en", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "dM", "11/10 - 11/20", 
        
        "en", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "My", "11/2007", 
        
        "en", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "EdM", "Sat, 11/10 - Tue, 11/20", 
        
        "en", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "d", "10 - 20", 
        
        "en", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "Ed", "10 Sat - 20 Tue", 
        
        "en", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "y", "2007", 
        
        "en", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "M", "11", 
        
        "en", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "MMM", "Nov", 
        
        "en", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "MMMM", "November", 
        
        "en", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "hm", "11/10/2007, 10:10 AM - 11/20/2007, 10:10 AM", 
        "en", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "Hm", "11/10/2007, 10:10 - 11/20/2007, 10:10", 
        "en", "2007 11 10 20:10:10", "2007 11 20 20:10:10", "Hm", "11/10/2007, 20:10 - 11/20/2007, 20:10", 
        
        "en", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "hmv", "11/10/2007, 10:10 AM PT - 11/20/2007, 10:10 AM PT", 
        
        "en", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "hmz", "11/10/2007, 10:10 AM PST - 11/20/2007, 10:10 AM PST", 
        
        "en", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "h", "11/10/2007, 10 AM - 11/20/2007, 10 AM", 
        
        "en", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "hv", "11/10/2007, 10 AM PT - 11/20/2007, 10 AM PT", 
        
        "en", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "hz", "11/10/2007, 10 AM PST - 11/20/2007, 10 AM PST", 
        
        "en", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "EEddMMyyyy", "Sat, 11/10/2007 - Tue, 11/20/2007", 
        
        "en", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "EddMMy", "Sat, 11/10/2007 - Tue, 11/20/2007", 
        
        "en", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "hhmm", "11/10/2007, 10:10 AM - 11/20/2007, 10:10 AM", 
        
        "en", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "hhmmzz", "11/10/2007, 10:10 AM PST - 11/20/2007, 10:10 AM PST", 
        
        "en", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "hms", "11/10/2007, 10:10:10 AM - 11/20/2007, 10:10:10 AM", 
        "en", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "Hms", "11/10/2007, 10:10:10 - 11/20/2007, 10:10:10", 
        "en", "2007 11 10 20:10:10", "2007 11 20 20:10:10", "Hms", "11/10/2007, 20:10:10 - 11/20/2007, 20:10:10", 
        
        "en", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "dMMMMMy", "N 10 - 20, 2007", 
        
        "en", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "EEEEEdM", "S, 11/10 - T, 11/20", 
        
        "en", "2007 01 10 10:00:10", "2007 01 10 14:10:10", "EEEEdMMMMy", "Wednesday, January 10, 2007", 
        
        "en", "2007 01 10 10:00:10", "2007 01 10 14:10:10", "dMMMMy", "January 10, 2007", 
        
        "en", "2007 01 10 10:00:10", "2007 01 10 14:10:10", "dMMMM", "January 10", 
        
        "en", "2007 01 10 10:00:10", "2007 01 10 14:10:10", "MMMMy", "January 2007", 
        
        "en", "2007 01 10 10:00:10", "2007 01 10 14:10:10", "EEEEdMMMM", "Wednesday, January 10", 
        
        "en", "2007 01 10 10:00:10", "2007 01 10 14:10:10", "EdMMMy", "Wed, Jan 10, 2007", 
        
        "en", "2007 01 10 10:00:10", "2007 01 10 14:10:10", "dMMMy", "Jan 10, 2007", 
        
        "en", "2007 01 10 10:00:10", "2007 01 10 14:10:10", "dMMM", "Jan 10", 
        
        "en", "2007 01 10 10:00:10", "2007 01 10 14:10:10", "MMMy", "Jan 2007", 
        
        "en", "2007 01 10 10:00:10", "2007 01 10 14:10:10", "EdMMM", "Wed, Jan 10", 
        
        
        "en", "2007 01 10 10:00:10", "2007 01 10 14:10:10", "dMy", "1/10/2007", 
        
        "en", "2007 01 10 10:00:10", "2007 01 10 14:10:10", "dM", "1/10", 
        
        "en", "2007 01 10 10:00:10", "2007 01 10 14:10:10", "My", "1/2007", 
        
        "en", "2007 01 10 10:00:10", "2007 01 10 14:10:10", "EdM", "Wed, 1/10", 
        
        "en", "2007 01 10 10:00:10", "2007 01 10 14:10:10", "d", "10", 
        
        "en", "2007 01 10 10:00:10", "2007 01 10 14:10:10", "Ed", "10 Wed", 
        
        "en", "2007 01 10 10:00:10", "2007 01 10 14:10:10", "y", "2007", 
        
        "en", "2007 01 10 10:00:10", "2007 01 10 14:10:10", "M", "1", 
        
        "en", "2007 01 10 10:00:10", "2007 01 10 14:10:10", "MMM", "Jan", 
        
        "en", "2007 01 10 10:00:10", "2007 01 10 14:10:10", "MMMM", "January", 
        
        "en", "2007 01 10 10:00:10", "2007 01 10 14:10:10", "hm", "10:00 AM - 2:10 PM", 
        "en", "2007 01 10 10:00:10", "2007 01 10 14:10:10", "Hm", "10:00 - 14:10", 
        
        "en", "2007 01 10 10:00:10", "2007 01 10 14:10:10", "hmv", "10:00 AM - 2:10 PM PT", 
        
        "en", "2007 01 10 10:00:10", "2007 01 10 14:10:10", "hmz", "10:00 AM - 2:10 PM PST", 
        
        "en", "2007 01 10 10:00:10", "2007 01 10 14:10:10", "h", "10 AM - 2 PM", 
        "en", "2007 01 10 10:00:10", "2007 01 10 14:10:10", "H", "10 - 14", 
        
        
        "en", "2007 01 10 10:00:10", "2007 01 10 14:10:10", "hz", "10 AM - 2 PM PST", 
        
        "en", "2007 01 10 10:00:10", "2007 01 10 14:10:10", "EEddMMyyyy", "Wed, 01/10/2007", 
        
        "en", "2007 01 10 10:00:10", "2007 01 10 14:10:10", "EddMMy", "Wed, 01/10/2007", 
        
        "en", "2007 01 10 10:00:10", "2007 01 10 14:10:10", "hhmm", "10:00 AM - 2:10 PM", 
        "en", "2007 01 10 10:00:10", "2007 01 10 14:10:10", "HHmm", "10:00 - 14:10", 
        
        "en", "2007 01 10 10:00:10", "2007 01 10 14:10:10", "hhmmzz", "10:00 AM - 2:10 PM PST", 
        
        "en", "2007 01 10 10:00:10", "2007 01 10 14:10:10", "hms", "10:00:10 AM - 2:10:10 PM", 
        "en", "2007 01 10 10:00:10", "2007 01 10 14:10:10", "Hms", "10:00:10 - 14:10:10", 
        
        "en", "2007 01 10 10:00:10", "2007 01 10 14:10:10", "dMMMMMy", "J 10, 2007", 
        
        "en", "2007 01 10 10:00:10", "2007 01 10 14:10:10", "EEEEEdM", "W, 1/10", 
        "en", "2007 01 10 10:00:10", "2007 01 10 10:20:10", "dMMMMy", "January 10, 2007", 
        
        "en", "2007 01 10 10:00:10", "2007 01 10 10:20:10", "dMMMM", "January 10", 
        
        "en", "2007 01 10 10:00:10", "2007 01 10 10:20:10", "MMMMy", "January 2007", 
        
        "en", "2007 01 10 10:00:10", "2007 01 10 10:20:10", "EEEEdMMMM", "Wednesday, January 10", 
        
        "en", "2007 01 10 10:00:10", "2007 01 10 10:20:10", "EdMMMy", "Wed, Jan 10, 2007", 
        
        "en", "2007 01 10 10:00:10", "2007 01 10 10:20:10", "dMMMy", "Jan 10, 2007", 
        
        "en", "2007 01 10 10:00:10", "2007 01 10 10:20:10", "dMMM", "Jan 10", 
        
        
        "en", "2007 01 10 10:00:10", "2007 01 10 10:20:10", "EdMMM", "Wed, Jan 10", 
        
        "en", "2007 01 10 10:00:10", "2007 01 10 10:20:10", "EdMy", "Wed, 1/10/2007", 
        
        "en", "2007 01 10 10:00:10", "2007 01 10 10:20:10", "dMy", "1/10/2007", 
        
        
        "en", "2007 01 10 10:00:10", "2007 01 10 10:20:10", "My", "1/2007", 
        
        "en", "2007 01 10 10:00:10", "2007 01 10 10:20:10", "EdM", "Wed, 1/10", 
        
        "en", "2007 01 10 10:00:10", "2007 01 10 10:20:10", "d", "10", 
        
        
        "en", "2007 01 10 10:00:10", "2007 01 10 10:20:10", "y", "2007", 
        
        "en", "2007 01 10 10:00:10", "2007 01 10 10:20:10", "M", "1", 
        
        
        
        "en", "2007 01 10 10:00:10", "2007 01 10 10:20:10", "hm", "10:00 - 10:20 AM", 
        "en", "2007 01 10 10:00:10", "2007 01 10 10:20:10", "Hm", "10:00 - 10:20", 
        
        
        "en", "2007 01 10 10:00:10", "2007 01 10 10:20:10", "hmz", "10:00 - 10:20 AM PST", 
        
        
        "en", "2007 01 10 10:00:10", "2007 01 10 10:20:10", "hv", "10 AM PT", 
        
        
        
        "en", "2007 01 10 10:00:10", "2007 01 10 10:20:10", "EddMMy", "Wed, 01/10/2007", 
        
        "en", "2007 01 10 10:00:10", "2007 01 10 10:20:10", "hhmm", "10:00 - 10:20 AM", 
        "en", "2007 01 10 10:00:10", "2007 01 10 10:20:10", "HHmm", "10:00 - 10:20", 
        
        "en", "2007 01 10 10:00:10", "2007 01 10 10:20:10", "hhmmzz", "10:00 - 10:20 AM PST", 
        
        
        "en", "2007 01 10 10:00:10", "2007 01 10 10:20:10", "dMMMMMy", "J 10, 2007", 
        
        
        "en", "2007 01 10 10:10:10", "2007 01 10 10:10:20", "EEEEdMMMMy", "Wednesday, January 10, 2007", 
        
        "en", "2007 01 10 10:10:10", "2007 01 10 10:10:20", "dMMMMy", "January 10, 2007", 
        
        
        "en", "2007 01 10 10:10:10", "2007 01 10 10:10:20", "MMMMy", "January 2007", 
        
        "en", "2007 01 10 10:10:10", "2007 01 10 10:10:20", "EEEEdMMMM", "Wednesday, January 10", 
        
        
        "en", "2007 01 10 10:10:10", "2007 01 10 10:10:20", "dMMMy", "Jan 10, 2007", 
        
        "en", "2007 01 10 10:10:10", "2007 01 10 10:10:20", "dMMM", "Jan 10", 
        
        
        "en", "2007 01 10 10:10:10", "2007 01 10 10:10:20", "EdMMM", "Wed, Jan 10", 
        
        "en", "2007 01 10 10:10:10", "2007 01 10 10:10:20", "EdMy", "Wed, 1/10/2007", 
        
        "en", "2007 01 10 10:10:10", "2007 01 10 10:10:20", "dMy", "1/10/2007", 
        
        
        "en", "2007 01 10 10:10:10", "2007 01 10 10:10:20", "My", "1/2007", 
        
        "en", "2007 01 10 10:10:10", "2007 01 10 10:10:20", "EdM", "Wed, 1/10", 
        
        "en", "2007 01 10 10:10:10", "2007 01 10 10:10:20", "d", "10", 
        
        
        "en", "2007 01 10 10:10:10", "2007 01 10 10:10:20", "y", "2007", 
        
        "en", "2007 01 10 10:10:10", "2007 01 10 10:10:20", "M", "1", 
        
        
        "en", "2007 01 10 10:10:10", "2007 01 10 10:10:20", "MMMM", "January", 
        
        "en", "2007 01 10 10:10:10", "2007 01 10 10:10:20", "hm", "10:10 AM", 
        "en", "2007 01 10 10:10:10", "2007 01 10 10:10:20", "Hm", "10:10", 
        
        
        "en", "2007 01 10 10:10:10", "2007 01 10 10:10:20", "hmz", "10:10 AM PST", 
        
        "en", "2007 01 10 10:10:10", "2007 01 10 10:10:20", "h", "10 AM", 
        
        "en", "2007 01 10 10:10:10", "2007 01 10 10:10:20", "hv", "10 AM PT", 
        
        
        "en", "2007 01 10 10:10:10", "2007 01 10 10:10:20", "EEddMMyyyy", "Wed, 01/10/2007", 
        
        
        "en", "2007 01 10 10:10:10", "2007 01 10 10:10:20", "hhmm", "10:10 AM", 
        "en", "2007 01 10 10:10:10", "2007 01 10 10:10:20", "HHmm", "10:10", 
        
        "en", "2007 01 10 10:10:10", "2007 01 10 10:10:20", "hhmmzz", "10:10 AM PST", 
        
        
        "en", "2007 01 10 10:10:10", "2007 01 10 10:10:20", "dMMMMMy", "J 10, 2007", 
        
        "en", "2007 01 10 10:10:10", "2007 01 10 10:10:20", "EEEEEdM", "W, 1/10", 
        
        "zh", "2007 10 10 10:10:10", "2008 10 10 10:10:10", "EEEEdMMMMy", "2007\\u5e7410\\u670810\\u65e5\\u661f\\u671f\\u4e09\\u81f32008\\u5e7410\\u670810\\u65e5\\u661f\\u671f\\u4e94", 
        
        
        "zh", "2007 10 10 10:10:10", "2007 11 10 10:10:10", "dMMMMy", "2007\\u5e7410\\u670810\\u65e5\\u81f311\\u670810\\u65e5", 
        
        
        "zh", "2007 10 10 10:10:10", "2007 11 10 10:10:10", "MMMMy", "2007\\u5e7410\\u6708\\u81f311\\u6708", 
        
        
        "zh", "2007 10 10 10:10:10", "2007 11 10 10:10:10", "hmv", "2007/10/10 \\u6D1B\\u6749\\u77F6\\u65F6\\u95F4\\u4E0A\\u534810:10 \\u2013 2007/11/10 \\u6D1B\\u6749\\u77F6\\u65F6\\u95F4\\u4E0A\\u534810:10", 
        
        "zh", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "EEEEdMMMMy", "2007\\u5e7411\\u670810\\u65e5\\u661f\\u671f\\u516d\\u81f320\\u65e5\\u661f\\u671f\\u4e8c", 
        
        
        "zh", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "dMMMM", "11\\u670810\\u65e5\\u81f320\\u65e5", 
        
        "zh", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "MMMMy", "2007\\u5E7411\\u6708", // (fixed expected result per ticket:6626:)
        
        "zh", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "EEEEdMMMM", "11\\u670810\\u65e5\\u661f\\u671f\\u516d\\u81f320\\u65e5\\u661f\\u671f\\u4e8c", 
        
        
        "zh", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "EdMy", "2007/11/10\\u5468\\u516d\\u81f32007/11/20\\u5468\\u4e8c", 
        
        
        "zh", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "dM", "11/10 \\u2013 11/20", 
        
        "zh", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "My", "2007/11", 
        
        "zh", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "EdM", "11/10\\u5468\\u516d\\u81f311/20\\u5468\\u4e8c", 
        
        
        "zh", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "y", "2007\\u5E74", // (fixed expected result per ticket:6626:)
        
        "zh", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "M", "11\\u6708", 
        
        "zh", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "MMM", "11\\u6708", // (fixed expected result per ticket:6626: and others)
        
        
        "zh", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "hmz", "2007/11/10 GMT-8\\u4e0a\\u534810:10 \\u2013 2007/11/20 GMT-8\\u4e0a\\u534810:10", 
        
        "zh", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "h", "2007/11/10 \\u4e0a\\u534810\\u65f6 \\u2013 2007/11/20 \\u4e0a\\u534810\\u65f6", 
        
        "zh", "2007 01 10 10:00:10", "2007 01 10 14:10:10", "EEEEdMMMMy", "2007\\u5e741\\u670810\\u65e5 \\u661f\\u671f\\u4e09", // (fixed expected result per ticket:6626:)
        
        "zh", "2007 01 10 10:00:10", "2007 01 10 14:10:10", "hm", "\\u4e0a\\u534810:00\\u81f3\\u4e0b\\u53482:10", 
        
        
        "zh", "2007 01 10 10:00:10", "2007 01 10 14:10:10", "hmz", "GMT-8\\u4e0a\\u534810:00\\u81f3\\u4e0b\\u53482:10", 
        
        "zh", "2007 01 10 10:00:10", "2007 01 10 14:10:10", "h", "\\u4e0a\\u534810\\u65F6\\u81f3\\u4e0b\\u53482\\u65f6", 
        
        "zh", "2007 01 10 10:00:10", "2007 01 10 14:10:10", "hv", "\\u6D1B\\u6749\\u77F6\\u65F6\\u95F4\\u4E0A\\u534810\\u65F6\\u81F3\\u4E0B\\u53482\\u65F6",
        
        "zh", "2007 01 10 10:00:10", "2007 01 10 10:20:10", "hm", "\\u4e0a\\u534810:00\\u81f310:20", 
        
        "zh", "2007 01 10 10:00:10", "2007 01 10 10:20:10", "hmv", "\\u6D1B\\u6749\\u77F6\\u65F6\\u95F4\\u4E0A\\u534810:00\\u81F310:20",
        
        "zh", "2007 01 10 10:00:10", "2007 01 10 10:20:10", "hz", "GMT-8\\u4e0a\\u534810\\u65f6", 
        
        "zh", "2007 01 10 10:10:10", "2007 01 10 10:10:20", "hm", "\\u4e0a\\u534810:10", 
        
        "zh", "2007 01 10 10:10:10", "2007 01 10 10:10:20", "h", "\\u4e0a\\u534810\\u65f6", 
        
        "de", "2007 10 10 10:10:10", "2008 10 10 10:10:10", "EEEEdMMMy", "Mittwoch, 10. Okt. 2007 - Freitag, 10. Okt. 2008", 
        
        
        "de", "2007 10 10 10:10:10", "2008 10 10 10:10:10", "dMMM", "10. Okt. 2007 - 10. Okt. 2008", 
        
        "de", "2007 10 10 10:10:10", "2008 10 10 10:10:10", "MMMy", "Okt. 2007 - Okt. 2008", 
        
        
        "de", "2007 10 10 10:10:10", "2008 10 10 10:10:10", "EdMy", "Mi., 10.10.2007 - Fr., 10.10.2008", 
        
        "de", "2007 10 10 10:10:10", "2008 10 10 10:10:10", "dMy", "10.10.2007 - 10.10.2008", 
        
        
        "de", "2007 10 10 10:10:10", "2008 10 10 10:10:10", "My", "10.2007 - 10.2008", 
        
        "de", "2007 10 10 10:10:10", "2008 10 10 10:10:10", "EdM", "Mi., 10.10.2007 - Fr., 10.10.2008", 
        
        
        "de", "2007 10 10 10:10:10", "2008 10 10 10:10:10", "y", "2007-2008", 
        
        "de", "2007 10 10 10:10:10", "2008 10 10 10:10:10", "M", "10.2007 - 10.2008", 
        
        
        "de", "2007 10 10 10:10:10", "2008 10 10 10:10:10", "hm", "10.10.2007 10:10 vorm. - 10.10.2008 10:10 vorm.", 
        "de", "2007 10 10 10:10:10", "2008 10 10 10:10:10", "Hm", "10.10.2007 10:10 - 10.10.2008 10:10", 
        
        "de", "2007 10 10 10:10:10", "2007 11 10 10:10:10", "EEEEdMMMy", "Mittwoch, 10. Okt. - Samstag, 10. Nov. 2007", 
        
        
        "de", "2007 10 10 10:10:10", "2007 11 10 10:10:10", "dMMM", "10. Okt. - 10. Nov.", 
        
        "de", "2007 10 10 10:10:10", "2007 11 10 10:10:10", "MMMy", "Okt.-Nov. 2007", 
        
        "de", "2007 10 10 10:10:10", "2007 11 10 10:10:10", "EEEEdMMM", "Mittwoch, 10. Okt. - Samstag, 10. Nov.", 
        
        
        "de", "2007 10 10 10:10:10", "2007 11 10 10:10:10", "dM", "10.10. - 10.11.", 
        
        "de", "2007 10 10 10:10:10", "2007 11 10 10:10:10", "My", "10.2007 - 11.2007", 
        
        
        "de", "2007 10 10 10:10:10", "2007 11 10 10:10:10", "d", "10.10. - 10.11.", 
        
        "de", "2007 10 10 10:10:10", "2007 11 10 10:10:10", "y", "2007", 
        
        
        "de", "2007 10 10 10:10:10", "2007 11 10 10:10:10", "MMM", "Okt.-Nov.", 
        
        
        "de", "2007 10 10 10:10:10", "2007 11 10 10:10:10", "hms", "10.10.2007 10:10:10 vorm. - 10.11.2007 10:10:10 vorm.", 
        "de", "2007 10 10 10:10:10", "2007 11 10 10:10:10", "Hms", "10.10.2007 10:10:10 - 10.11.2007 10:10:10", 
        
        "de", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "EEEEdMMMy", "Samstag, 10. - Dienstag, 20. Nov. 2007", 
        
        "de", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "dMMMy", "10.-20. Nov. 2007", 
        
        
        "de", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "MMMy", "Nov. 2007", 
        
        "de", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "EEEEdMMM", "Samstag, 10. - Dienstag, 20. Nov.", 
        
        "de", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "EdMy", "Sa., 10.11.2007 - Di., 20.11.2007", 
        
        
        "de", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "dM", "10.11. - 20.11.", 
        
        "de", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "My", "11.2007", 
        
        
        "de", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "d", "10.-20.", 
        
        "de", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "y", "2007", 
        
        
        "de", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "hmv", "10.11.2007 10:10 vorm. Los Angeles Zeit - 20.11.2007 10:10 vorm. Los Angeles Zeit", 
        
        "de", "2007 01 10 10:00:10", "2007 01 10 14:10:10", "EEEEdMMMy", "Mittwoch, 10. Jan. 2007", 
        
        
        "de", "2007 01 10 10:00:10", "2007 01 10 14:10:10", "dMMM", "10. Jan.", 
        
        "de", "2007 01 10 10:00:10", "2007 01 10 14:10:10", "MMMy", "Jan. 2007", 
        
        "de", "2007 01 10 10:00:10", "2007 01 10 14:10:10", "EEEEdMMM", "Mittwoch, 10. Jan.", 
        
        
        "de", "2007 01 10 10:00:10", "2007 01 10 14:10:10", "h", "10 vorm. - 2 nachm.", 
        "de", "2007 01 10 10:00:10", "2007 01 10 14:10:10", "H", "10-14 Uhr", 
        
        "de", "2007 01 10 10:00:10", "2007 01 10 10:20:10", "EEEEdMMM", "Mittwoch, 10. Jan.", 
        
        
        "de", "2007 01 10 10:00:10", "2007 01 10 10:20:10", "hmv", "10:00-10:20 vorm. Los Angeles Zeit", 
        
        "de", "2007 01 10 10:00:10", "2007 01 10 10:20:10", "hmz", "10:00-10:20 vorm. GMT-8", 
        
        "de", "2007 01 10 10:00:10", "2007 01 10 10:20:10", "h", "10 vorm.", 
        "de", "2007 01 10 10:00:10", "2007 01 10 10:20:10", "H", "10 Uhr", 
        
        
        "de", "2007 01 10 10:00:10", "2007 01 10 10:20:10", "hz", "10 vorm. GMT-8", 
        
        "de", "2007 01 10 10:10:10", "2007 01 10 10:10:20", "EEEEdMMMy", "Mittwoch, 10. Jan. 2007", 
        
        
        "de", "2007 01 10 10:10:10", "2007 01 10 10:10:20", "hmv", "10:10 vorm. Los Angeles Zeit", 
        
        "de", "2007 01 10 10:10:10", "2007 01 10 10:10:20", "hmz", "10:10 vorm. GMT-8", 
        
        
        "de", "2007 01 10 10:10:10", "2007 01 10 10:10:20", "hv", "10 vorm. Los Angeles Zeit", 
        
        "de", "2007 01 10 10:10:10", "2007 01 10 10:10:20", "hz", "10 vorm. GMT-8", 
        
        // Thai (default calendar buddhist)

        "th", "2550 10 10 10:10:10", "2551 10 10 10:10:10", "EEEEdMMMy", "\\u0E27\\u0E31\\u0E19\\u0E1E\\u0E38\\u0E18 10 \\u0E15.\\u0E04. 2550 - \\u0E27\\u0E31\\u0E19\\u0E28\\u0E38\\u0E01\\u0E23\\u0E4C 10 \\u0E15.\\u0E04. 2551", 
        
        
        "th", "2550 10 10 10:10:10", "2551 10 10 10:10:10", "dMMM", "10 \\u0E15.\\u0E04. 2550 - 10 \\u0E15.\\u0E04. 2551", 
        
        "th", "2550 10 10 10:10:10", "2551 10 10 10:10:10", "MMMy", "\\u0E15.\\u0E04. 2550 - \\u0E15.\\u0E04. 2551", 
        
        
        "th", "2550 10 10 10:10:10", "2551 10 10 10:10:10", "EdMy", "\\u0E1E. 10/10/2550 - \\u0E28. 10/10/2551", 
        
        "th", "2550 10 10 10:10:10", "2551 10 10 10:10:10", "dMy", "10/10/2550 - 10/10/2551", 
        
        
        "th", "2550 10 10 10:10:10", "2551 10 10 10:10:10", "My", "10/2550 - 10/2551", 
        
        "th", "2550 10 10 10:10:10", "2551 10 10 10:10:10", "EdM", "\\u0E1E. 10/10/2550 - \\u0E28. 10/10/2551", 
        
        
        "th", "2550 10 10 10:10:10", "2551 10 10 10:10:10", "y", "2550-2551", 
        
        "th", "2550 10 10 10:10:10", "2551 10 10 10:10:10", "M", "10/2550 - 10/2551", 
        
        
        "th", "2550 10 10 10:10:10", "2550 11 10 10:10:10", "EEEEdMMMy", "\\u0E27\\u0E31\\u0E19\\u0E1E\\u0E38\\u0E18 10 \\u0E15.\\u0E04. - \\u0E27\\u0E31\\u0E19\\u0E40\\u0E2A\\u0E32\\u0E23\\u0E4C 10 \\u0E1E.\\u0E22. 2550", 
        
        
        "th", "2550 10 10 10:10:10", "2550 11 10 10:10:10", "dMMM", "10 \\u0E15.\\u0E04. - 10 \\u0E1E.\\u0E22.", 
        
        "th", "2550 10 10 10:10:10", "2550 11 10 10:10:10", "MMMy", "\\u0E15.\\u0E04.-\\u0E1E.\\u0E22. 2550", 
        
        "th", "2550 10 10 10:10:10", "2550 11 10 10:10:10", "dM", "10/10 - 10/11", 
        
        "th", "2550 10 10 10:10:10", "2550 11 10 10:10:10", "My", "10/2550 - 11/2550", 
        
        
        "th", "2550 10 10 10:10:10", "2550 11 10 10:10:10", "d", "10/10 - 10/11", 
        
        "th", "2550 10 10 10:10:10", "2550 11 10 10:10:10", "y", "\\u0E1E.\\u0E28. 2550", 
        
        
        "th", "2550 10 10 10:10:10", "2550 11 10 10:10:10", "MMM", "\\u0E15.\\u0E04.-\\u0E1E.\\u0E22.", 

    };
    expect(DATA, ARRAY_SIZE(DATA));
}


void DateIntervalFormatTest::expect(const char** data, int32_t data_length) {
    int32_t i = 0;
    UErrorCode ec = U_ZERO_ERROR;
    UnicodeString str, str2;
    const char* pattern = data[0];
    i++;

#ifdef DTIFMTTS_DEBUG
    char result[1000]; 
    char mesg[1000];
    sprintf(mesg, "locale: %s\n", locName);  
    PRINTMESG(mesg);
#endif

    while (i<data_length) {
        const char* locName = data[i++];
        Locale loc(locName);
        SimpleDateFormat ref(pattern, loc, ec);
        if (U_FAILURE(ec)) {
            dataerrln("contruct SimpleDateFormat in expect failed: %s", u_errorName(ec));
            return;
        }
        // 'f'
        const char* datestr = data[i++];
        const char* datestr_2 = data[i++];
#ifdef DTIFMTTS_DEBUG
        sprintf(mesg, "original date: %s - %s\n", datestr, datestr_2);
        PRINTMESG(mesg)
#endif
        UDate date = ref.parse(ctou(datestr), ec);
        if (!assertSuccess("parse 1st data in expect", ec)) return;
        UDate date_2 = ref.parse(ctou(datestr_2), ec);
        if (!assertSuccess("parse 2nd data in expect", ec)) return;
        DateInterval dtitv(date, date_2);

        const UnicodeString& oneSkeleton = data[i++];

        DateIntervalFormat* dtitvfmt = DateIntervalFormat::createInstance(oneSkeleton, loc, ec);
        if (!assertSuccess("createInstance(skeleton) in expect", ec)) return;
        FieldPosition pos=0;
        dtitvfmt->format(&dtitv, str.remove(), pos, ec);
        if (!assertSuccess("format in expect", ec)) return;
        assertEquals((UnicodeString)"\"" + locName + "\\" + oneSkeleton + "\\" + datestr + "\\" + datestr_2 + "\"", ctou(data[i++]), str);



#ifdef DTIFMTTS_DEBUG
        str.extract(0,  str.length(), result, "UTF-8");
        sprintf(mesg, "interval date: %s\n", result);
        std::cout << "//";
        PRINTMESG(mesg)
        std::cout << "\"" << locName << "\", " 
                 << "\"" << datestr << "\", " 
                 << "\"" << datestr_2 << "\", ";
        printUnicodeString(oneSkeleton);
        printUnicodeString(str);
        std::cout << "\n\n";        
#endif
        delete dtitvfmt;
    }
}


/* 
 * Test format using user defined DateIntervalInfo
 */
void DateIntervalFormatTest::testFormatUserDII() {
    // first item is date pattern
    const char* DATA[] = {
        "yyyy MM dd HH:mm:ss",    
        "en", "2007 10 10 10:10:10", "2008 10 10 10:10:10", "Oct 10, 2007 --- Oct 10, 2008", 
        
        "en", "2007 10 10 10:10:10", "2007 11 10 10:10:10", "2007 Oct 10 - Nov 2007", 
        
        "en", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "Nov 10, 2007 --- Nov 20, 2007", 
        
        "en", "2007 01 10 10:00:10", "2007 01 10 14:10:10", "Jan 10, 2007", 
        
        "en", "2007 01 10 10:00:10", "2007 01 10 10:20:10", "Jan 10, 2007", 
        
        "en", "2007 01 10 10:10:10", "2007 01 10 10:10:20", "Jan 10, 2007", 
        
        "zh", "2007 10 10 10:10:10", "2008 10 10 10:10:10", "2007\\u5e7410\\u670810\\u65e5 --- 2008\\u5e7410\\u670810\\u65e5", 
        
        "zh", "2007 10 10 10:10:10", "2007 11 10 10:10:10", "2007 10\\u6708 10 - 11\\u6708 2007", 
        
        "zh", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "2007\\u5e7411\\u670810\\u65e5 --- 2007\\u5e7411\\u670820\\u65e5", 
        
        "zh", "2007 01 10 10:00:10", "2007 01 10 14:10:10", "2007\\u5e741\\u670810\\u65e5", // (fixed expected result per ticket:6626:)
        
        "zh", "2007 01 10 10:00:10", "2007 01 10 10:20:10", "2007\\u5e741\\u670810\\u65e5", // (fixed expected result per ticket:6626:)
        
        "zh", "2007 01 10 10:10:10", "2007 01 10 10:10:20", "2007\\u5e741\\u670810\\u65e5", // (fixed expected result per ticket:6626:)
        
        "de", "2007 10 10 10:10:10", "2008 10 10 10:10:10", "10. Okt. 2007 --- 10. Okt. 2008", 
        
        
        "de", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "10. Nov. 2007 --- 20. Nov. 2007", 
        
        "de", "2007 01 10 10:00:10", "2007 01 10 14:10:10", "10. Jan. 2007", 
        
        "de", "2007 01 10 10:00:10", "2007 01 10 10:20:10", "10. Jan. 2007", 
        
        
        "es", "2007 10 10 10:10:10", "2008 10 10 10:10:10", "10 oct 2007 --- 10 oct 2008", 
        
        "es", "2007 10 10 10:10:10", "2007 11 10 10:10:10", "2007 oct 10 - nov 2007", 
        
        "es", "2007 11 10 10:10:10", "2007 11 20 10:10:10", "10 nov 2007 --- 20 nov 2007", 
        
        "es", "2007 01 10 10:00:10", "2007 01 10 14:10:10", "10 ene 2007", 
        
        "es", "2007 01 10 10:00:10", "2007 01 10 10:20:10", "10 ene 2007", 
       
        "es", "2007 01 10 10:10:10", "2007 01 10 10:10:20", "10 ene 2007", 
    };
    expectUserDII(DATA, ARRAY_SIZE(DATA));
}


void DateIntervalFormatTest::testSetIntervalPatternNoSideEffect() {
    UErrorCode ec = U_ZERO_ERROR;
    LocalPointer<DateIntervalInfo> dtitvinf(new DateIntervalInfo(ec));
    if (U_FAILURE(ec)) {
        errln("Failure encountered: %s", u_errorName(ec));
        return;
    }
    UnicodeString expected;
    dtitvinf->getIntervalPattern(ctou("yMd"), UCAL_DATE, expected, ec);
    dtitvinf->setIntervalPattern(ctou("yMd"), UCAL_DATE, ctou("M/d/y \\u2013 d"), ec);
    if (U_FAILURE(ec)) {
        errln("Failure encountered: %s", u_errorName(ec));
        return;
    }
    dtitvinf.adoptInstead(new DateIntervalInfo(ec));
    if (U_FAILURE(ec)) {
        errln("Failure encountered: %s", u_errorName(ec));
        return;
    }
    UnicodeString actual;
    dtitvinf->getIntervalPattern(ctou("yMd"), UCAL_DATE, actual, ec);
    if (U_FAILURE(ec)) {
        errln("Failure encountered: %s", u_errorName(ec));
        return;
    }
    if (expected != actual) {
        errln("DateIntervalInfo.setIntervalPattern should have no side effects.");
    }
}

void DateIntervalFormatTest::testYearFormats() {
    const Locale &enLocale = Locale::getEnglish();
    UErrorCode status = U_ZERO_ERROR;
    LocalPointer<Calendar> fromTime(Calendar::createInstance(enLocale, status));
    LocalPointer<Calendar> toTime(Calendar::createInstance(enLocale, status));
    if (U_FAILURE(status)) {
        errln("Failure encountered: %s", u_errorName(status));
        return;
    }
    // April 26, 113. Three digit year so that we can test 2 digit years;
    // 4 digit years with padded 0's and full years.
    fromTime->set(113, 3, 26);
    // April 28, 113.
    toTime->set(113, 3, 28);
    {
        LocalPointer<DateIntervalFormat> dif(DateIntervalFormat::createInstance("yyyyMd", enLocale, status));
        if (U_FAILURE(status)) {
            dataerrln("Failure encountered: %s", u_errorName(status));
            return;
        }
        UnicodeString actual;
        UnicodeString expected(ctou("4/26/0113 - 4/28/0113"));
        FieldPosition pos = 0;
        dif->format(*fromTime, *toTime, actual, pos, status);
        if (U_FAILURE(status)) {
            errln("Failure encountered: %s", u_errorName(status));
            return;
        }
        if (actual != expected) {
            errln("Expected " + expected + ", got: " + actual);
        }
    }
    {
        LocalPointer<DateIntervalFormat> dif(DateIntervalFormat::createInstance("yyMd", enLocale, status));
        if (U_FAILURE(status)) {
            errln("Failure encountered: %s", u_errorName(status));
            return;
        }
        UnicodeString actual;
        UnicodeString expected(ctou("4/26/13 - 4/28/13"));
        FieldPosition pos = 0;
        dif->format(*fromTime, *toTime, actual, pos, status);
        if (U_FAILURE(status)) {
            errln("Failure encountered: %s", u_errorName(status));
            return;
        }
        if (actual != expected) {
            errln("Expected " + expected + ", got: " + actual);
        }
    }
    {
        LocalPointer<DateIntervalFormat> dif(DateIntervalFormat::createInstance("yMd", enLocale, status));
        if (U_FAILURE(status)) {
            errln("Failure encountered: %s", u_errorName(status));
            return;
        }
        UnicodeString actual;
        UnicodeString expected(ctou("4/26/113 - 4/28/113"));
        FieldPosition pos = 0;
        dif->format(*fromTime, *toTime, actual, pos, status);
        if (U_FAILURE(status)) {
            errln("Failure encountered: %s", u_errorName(status));
            return;
        }
        if (actual != expected) {
            errln("Expected " + expected + ", got: " + actual);
        }
    }
}

void DateIntervalFormatTest::expectUserDII(const char** data, 
                                           int32_t data_length) {
    int32_t i = 0;
    UnicodeString str;
    UErrorCode ec = U_ZERO_ERROR;
    const char* pattern = data[0];
    i++;

    while ( i < data_length ) {
        const char* locName = data[i++];
        Locale loc(locName);
        SimpleDateFormat ref(pattern, loc, ec);
        if (U_FAILURE(ec)) {
            dataerrln("contruct SimpleDateFormat in expectUserDII failed: %s", u_errorName(ec));
            return;
        }
        const char* datestr = data[i++];
        const char* datestr_2 = data[i++];
        UDate date = ref.parse(ctou(datestr), ec);
        if (!assertSuccess("parse in expectUserDII", ec)) return;
        UDate date_2 = ref.parse(ctou(datestr_2), ec);
        if (!assertSuccess("parse in expectUserDII", ec)) return;
        DateInterval dtitv(date, date_2);

        ec = U_ZERO_ERROR;
        // test user created DateIntervalInfo
        DateIntervalInfo* dtitvinf = new DateIntervalInfo(ec);
        dtitvinf->setFallbackIntervalPattern("{0} --- {1}", ec);
        dtitvinf->setIntervalPattern(UDAT_YEAR_ABBR_MONTH_DAY, UCAL_MONTH, "yyyy MMM d - MMM y",ec);
        if (!assertSuccess("DateIntervalInfo::setIntervalPattern", ec)) return;
        dtitvinf->setIntervalPattern(UDAT_YEAR_ABBR_MONTH_DAY, UCAL_HOUR_OF_DAY, "yyyy MMM d HH:mm - HH:mm", ec);
        if (!assertSuccess("DateIntervalInfo::setIntervalPattern", ec)) return;
        DateIntervalFormat* dtitvfmt = DateIntervalFormat::createInstance(UDAT_YEAR_ABBR_MONTH_DAY, loc, *dtitvinf, ec);
        delete dtitvinf;
        if (!assertSuccess("createInstance(skeleton,dtitvinf) in expectUserDII", ec)) return;
        FieldPosition pos=0;
        dtitvfmt->format(&dtitv, str.remove(), pos, ec);
        if (!assertSuccess("format in expectUserDII", ec)) return;
        assertEquals((UnicodeString)"\"" + locName + "\\" + datestr + "\\" + datestr_2 + "\"", ctou(data[i++]), str);
#ifdef DTIFMTTS_DEBUG
        char result[1000]; 
        char mesg[1000];
        PRINTMESG("interval format using user defined DateIntervalInfo\n");
        str.extract(0,  str.length(), result, "UTF-8");
        sprintf(mesg, "interval date: %s\n", result);
        PRINTMESG(mesg);
#endif
        delete dtitvfmt;
    }
}


void DateIntervalFormatTest::testStress() {
    if(quick){
    	logln("Quick mode: Skipping test");
    	return;
    }
	const char* DATA[] = {
        "yyyy MM dd HH:mm:ss",
        "2007 10 10 10:10:10", "2008 10 10 10:10:10", 
        "2007 10 10 10:10:10", "2007 11 10 10:10:10", 
        "2007 11 10 10:10:10", "2007 11 20 10:10:10", 
        "2007 01 10 10:00:10", "2007 01 10 14:10:10", 
        "2007 01 10 10:00:10", "2007 01 10 10:20:10", 
        "2007 01 10 10:10:10", "2007 01 10 10:10:20", 
    };

    const char* testLocale[][3] = {
        //{"th", "", ""},
        {"en", "", ""},
        {"zh", "", ""},
        {"de", "", ""},
        {"ar", "", ""},
        {"en", "GB",  ""},
        {"fr", "", ""},
        {"it", "", ""},
        {"nl", "", ""},
        {"zh", "TW",  ""},
        {"ja", "", ""},
        {"pt", "BR", ""},
        {"ru", "", ""},
        {"pl", "", ""},
        {"tr", "", ""},
        {"es", "", ""},
        {"ko", "", ""},
        {"sv", "", ""},
        {"fi", "", ""},
        {"da", "", ""},
        {"pt", "PT", ""},
        {"ro", "", ""},
        {"hu", "", ""},
        {"he", "", ""},
        {"in", "", ""},
        {"cs", "", ""},
        {"el", "", ""},
        {"no", "", ""},
        {"vi", "", ""},
        {"bg", "", ""},
        {"hr", "", ""},
        {"lt", "", ""},
        {"sk", "", ""},
        {"sl", "", ""},
        {"sr", "", ""},
        {"ca", "", ""},
        {"lv", "", ""},
        {"uk", "", ""},
        {"hi", "", ""},
    };

    uint32_t localeIndex;
    for ( localeIndex = 0; localeIndex < ARRAY_SIZE(testLocale); ++localeIndex ) {
        char locName[32];
        uprv_strcpy(locName, testLocale[localeIndex][0]);
        uprv_strcat(locName, testLocale[localeIndex][1]);
        stress(DATA, ARRAY_SIZE(DATA), Locale(testLocale[localeIndex][0], testLocale[localeIndex][1], testLocale[localeIndex][2]), locName);
    }
}


void DateIntervalFormatTest::stress(const char** data, int32_t data_length,
                                    const Locale& loc, const char* locName) {
    UnicodeString skeleton[] = {
        "EEEEdMMMMy",
        "dMMMMy",
        "dMMMM",
        "MMMMy",
        "EEEEdMMMM",
        "EdMMMy",
        "dMMMy",
        "dMMM",
        "MMMy",
        "EdMMM",
        "EdMy",
        "dMy",
        "dM",
        "My",
        "EdM",
        "d",
        "Ed",
        "y",
        "M",
        "MMM",
        "MMMM",
        "hm",
        "hmv",
        "hmz",
        "h",
        "hv",
        "hz",
        "EEddMMyyyy", // following could be normalized
        "EddMMy", 
        "hhmm",
        "hhmmzz",
        "hms",  // following could not be normalized
        "dMMMMMy",
        "EEEEEdM",
    };

    int32_t i = 0;
    UErrorCode ec = U_ZERO_ERROR;
    UnicodeString str, str2;
    SimpleDateFormat ref(data[i++], loc, ec);
    if (!assertSuccess("construct SimpleDateFormat", ec)) return;

#ifdef DTIFMTTS_DEBUG
    char result[1000]; 
    char mesg[1000];
    sprintf(mesg, "locale: %s\n", locName);  
    PRINTMESG(mesg);
#endif

    while (i<data_length) {

        // 'f'
        const char* datestr = data[i++];
        const char* datestr_2 = data[i++];
#ifdef DTIFMTTS_DEBUG
        sprintf(mesg, "original date: %s - %s\n", datestr, datestr_2);
        PRINTMESG(mesg)
#endif
        UDate date = ref.parse(ctou(datestr), ec);
        if (!assertSuccess("parse", ec)) return;
        UDate date_2 = ref.parse(ctou(datestr_2), ec);
        if (!assertSuccess("parse", ec)) return;
        DateInterval dtitv(date, date_2);

        for ( uint32_t skeletonIndex = 0; 
              skeletonIndex < ARRAY_SIZE(skeleton); 
              ++skeletonIndex ) {
            const UnicodeString& oneSkeleton = skeleton[skeletonIndex];
            DateIntervalFormat* dtitvfmt = DateIntervalFormat::createInstance(oneSkeleton, loc, ec);
            if (!assertSuccess("createInstance(skeleton)", ec)) return;
            /*
            // reset the calendar to be Gregorian calendar for "th"
            if ( uprv_strcmp(locName, "th") == 0 ) {
                GregorianCalendar* gregCal = new GregorianCalendar(loc, ec);
                if (!assertSuccess("GregorianCalendar()", ec)) return;
                const DateFormat* dformat = dtitvfmt->getDateFormat();
                DateFormat* newOne = (DateFormat*)dformat->clone();
                newOne->adoptCalendar(gregCal);
                //dtitvfmt->adoptDateFormat(newOne, ec);
                dtitvfmt->setDateFormat(*newOne, ec);
                delete newOne;
                if (!assertSuccess("adoptDateFormat()", ec)) return;
            }
            */
            FieldPosition pos=0;
            dtitvfmt->format(&dtitv, str.remove(), pos, ec);
            if (!assertSuccess("format", ec)) return;
#ifdef DTIFMTTS_DEBUG
            oneSkeleton.extract(0,  oneSkeleton.length(), result, "UTF-8");
            sprintf(mesg, "interval by skeleton: %s\n", result);
            PRINTMESG(mesg)
            str.extract(0,  str.length(), result, "UTF-8");
            sprintf(mesg, "interval date: %s\n", result);
            PRINTMESG(mesg)
#endif
            delete dtitvfmt;
        }

        // test user created DateIntervalInfo
        ec = U_ZERO_ERROR;
        DateIntervalInfo* dtitvinf = new DateIntervalInfo(ec);
        dtitvinf->setFallbackIntervalPattern("{0} --- {1}", ec);
        dtitvinf->setIntervalPattern(UDAT_YEAR_ABBR_MONTH_DAY, UCAL_MONTH, "yyyy MMM d - MMM y",ec);
        if (!assertSuccess("DateIntervalInfo::setIntervalPattern", ec)) return;
        dtitvinf->setIntervalPattern(UDAT_YEAR_ABBR_MONTH_DAY, UCAL_HOUR_OF_DAY, "yyyy MMM d HH:mm - HH:mm", ec);
        if (!assertSuccess("DateIntervalInfo::setIntervalPattern", ec)) return;
        DateIntervalFormat* dtitvfmt = DateIntervalFormat::createInstance(UDAT_YEAR_ABBR_MONTH_DAY, loc, *dtitvinf, ec);
        delete dtitvinf;
        if (!assertSuccess("createInstance(skeleton,dtitvinf)", ec)) return;
        FieldPosition pos=0;
        dtitvfmt->format(&dtitv, str.remove(), pos, ec);
        if ( uprv_strcmp(locName, "th") ) {
            if (!assertSuccess("format", ec)) return;
#ifdef DTIFMTTS_DEBUG
            PRINTMESG("interval format using user defined DateIntervalInfo\n");
            str.extract(0,  str.length(), result, "UTF-8");
            sprintf(mesg, "interval date: %s\n", result);
            PRINTMESG(mesg)
#endif
        } else {
            // for "th", the default calendar is Budhist, 
            // not Gregorian.
            assertTrue("Default calendar for \"th\" is Budhist", ec == U_ILLEGAL_ARGUMENT_ERROR);
            ec = U_ZERO_ERROR;
        }
        delete dtitvfmt;
    }
}

#endif /* #if !UCONFIG_NO_FORMATTING */
