// This file is part of the SpeedCrunch project
// Copyright (C) 2016 Pol Welter <polwelter@gmail.com>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; see the file COPYING.  If not, write to
// the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
// Boston, MA 02110-1301, USA.

#include "math/quantity.h"
#include "math/units.h"

#include <QtCore/QCoreApplication>
#include <QString>

#include <cstdlib>
#include <cstring>
#include <iostream>

using namespace std;

#define CHECK(x,y) check_value(__FILE__,__LINE__,#x,x,y)
#define CHECK_FORMAT(f,p,x,y) check_format(__FILE__,__LINE__,#x,x,f,p,y)
#define CHECK_PRECISE(x,y) check_precise(__FILE__,__LINE__,#x,x,y)
#define CHECK_KNOWN_ISSUE(x,y,n) check_value(__FILE__,__LINE__,#x,x,y,n)

static int dmath_total_tests  = 0;
static int dmath_failed_tests = 0;
static int dmath_new_failed_tests = 0;

static void check_value(const char* file, int line, const char* msg, const Quantity& q, const QString& expected, int issue = 0)
{
    ++dmath_total_tests;
    QString result = DMath::format(q, 'f');
    if (result != expected) {
        ++dmath_failed_tests;
        cerr << file << "[" << line << "]\t" << msg;
        if (issue)
            cerr << "\t[ISSUE " << issue << "]";
        else {
            cerr << "\t[NEW]";
            ++dmath_new_failed_tests;
        }
        cerr << endl;
        cerr << "\tResult   : " << result.toLatin1().constData() << endl
             << "\tExpected : " << expected.toLatin1().constData() << endl;
    }
}

static void check_format(const char* file, int line, const char* msg, const Quantity& q, char format, int prec, const QString& expected)
{
    ++dmath_total_tests;
    QString result = DMath::format(q, format, prec);
    if (result != expected) {
        ++dmath_failed_tests;
        cerr << file << "[" << line << "]\t" << msg << endl;
        cerr << "\tResult   : " << result.toLatin1().constData() << endl
             << "\tExpected : " << expected.toLatin1().constData() << endl;
    }
}

static void check_precise(const char* file, int line, const char* msg, const Quantity& q, const QString& expected)
{
    ++dmath_total_tests;
    QString result = DMath::format(q, 'f', 50);
    if (result != expected) {
        ++dmath_failed_tests;
        cerr << file << "[" << line << "]: " << msg << endl
             << "\tResult  : " << result.toLatin1().constData() << endl
             << "\tExpected: " << expected.toLatin1().constData() << endl << endl;
    }
}

void test_create()
{
    CHECK(Units::meter(), "1 meter");
    CHECK(Quantity(CNumber("123.45+654j")), "123.45+654j");
    CHECK(Quantity(HNumber("123.45")), "123.45");
}

void test_basic()
{
    CHECK(Units::meter(), "1 meter");
    CHECK(Quantity(5) * Units::meter(), "5 meter");
    CHECK(Units::candela() + Units::second(), "NaN");
    CHECK(Quantity(3)*Units::mole() - Quantity(HNumber("2.5"))*Units::mole(), "0.5 mole");
    CHECK(Units::kilogram()/Units::second(), "1 kilogram second^-1");
    CHECK(Units::meter()*Units::meter(), "1 meter²");
    CHECK(-Quantity(5)*Units::meter(), "-5 meter");
    CNumber foot = CNumber("0.3");
    Quantity a(Quantity(123)*Units::meter());
    a.setDisplayUnit(foot, "foot");
    CHECK(a, "410 foot");
}

void test_functions()
{
    CHECK(DMath::abs(Quantity(CNumber("3+4j"))*Units::meter()), "5 meter");
    CHECK(DMath::round(CNumber("1.234"), 1), "1.2");
    CHECK(DMath::round(Quantity(CNumber("1.234"))*Units::joule(), 0), "NaN");

    CHECK(DMath::trunc(CNumber("1.274"), 1), "1.2");
    CHECK(DMath::trunc(Quantity(CNumber("1.234"))*Units::joule(), 0), "NaN");

    CHECK(DMath::real(Quantity(CNumber("3+4j"))*Units::meter()), "3 meter");
    CHECK(DMath::imag(Quantity(CNumber("3+4j"))*Units::meter()), "4 meter");

    CHECK(DMath::sqrt(Quantity(CNumber("36"))*Units::second()), "6 second^(1/2)");
    CHECK(DMath::cbrt(Quantity(CNumber("125"))*Units::second()), "5 second^(1/3)");

    CHECK(DMath::raise(Quantity(CNumber("2")), DMath::pi()), "8.82497782707628762386");
    CHECK(DMath::raise(Quantity(CNumber("2"))*Units::ampere(), DMath::pi()), "NaN");
    CHECK(DMath::raise(Quantity(CNumber("-2"))*Units::ampere(), Quantity(CNumber("1.5"))), "NaN");

    DMath::complexMode = false;
    CHECK(DMath::raise(Quantity(CNumber("-2"))*Units::ampere(), Quantity(CNumber("0.6"))), "-1.51571656651039808235 ampere^(3/5)");
    DMath::complexMode = true;
    CHECK(DMath::raise(Quantity(CNumber("-2"))*Units::ampere(), Quantity(CNumber("0.6"))), "(-0.46838217770735830743+1.44153211743623063689j) ampere^(3/5)");


    // this should do it for all wrapped functions that do not accept dimensional arguments...
    CHECK(DMath::sin(Quantity(DMath::pi())), "0");
    CHECK(DMath::sin(Units::meter()), "NaN");
}

void test_format()
{
    Quantity a = Quantity(CNumber("12365234.45647"));
    CHECK_FORMAT('b', 10, a, "0b101111001010110110110010.0111010011011011001101111100100110011010111010010010010011110010001");


    a *= Units::coulomb();
    CHECK_FORMAT('b', 10, a, "0b101111001010110110110010.0111010011011011001101111100100110011010111010010010010011110010001 coulomb");
}


int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    dmath_total_tests  = 0;
    dmath_failed_tests = 0;

    test_create();
    test_basic();
    test_functions();
    test_format();

    cout << dmath_total_tests  << " total, "
         << dmath_failed_tests << " failed, "
         << dmath_new_failed_tests << " new" << endl;

  return dmath_failed_tests;
}
