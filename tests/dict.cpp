/*  Embedis - Embedded Dictionary Server
    Copyright (C) 2015 PatternAgents, LLC

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "main.h"


extern "C" const embedis_ram_access mock_ram_access;

void mock_ram_erase() {
    size_t size = mock_ram_access.size();
    for (size_t i = 0; i < size; i++) mock_ram_access.store(i, 0xFF);
}


TEST(DictROM, Select) {

    embedis_init();
    mock_ram_erase();

    ASSERT_EMBEDIS_FAIL("SELECT this_does_not_exist");
    ASSERT_EMBEDIS_OK("SELECT ROM");
    EXPECT_EMBEDIS_STRING("GET vendor", "PatternAgents");
    ASSERT_EMBEDIS_OK("SELECT RAM");
    EXPECT_EMBEDIS_OK("DEL vendor");
    ASSERT_EMBEDIS_OK("SELECT rom");
    EXPECT_EMBEDIS_STRING("GET vendor", "PatternAgents");
}


TEST(DictROM, Basics) {

    embedis_init();

    EXPECT_EMBEDIS_OK("SELECT ROM");
    EXPECT_EMBEDIS_STRING("GET vendor", "PatternAgents");
    EXPECT_EMBEDIS_ERROR("SET vendor blah");
    EXPECT_EMBEDIS_ERROR("DEL vendor");
}


TEST(DictROM, Keys) {
    embedis_init();

    EXPECT_EMBEDIS_OK("SELECT ROM");
    EXPECT_EMBEDIS_ARRAY("KEYS", {"vendor"});
}


TEST(DictRAM, Basics) {
    std::string s;

    embedis_init();
    mock_ram_erase();

    EXPECT_EMBEDIS_OK("SELECT RAM");
    EXPECT_EMBEDIS_OK("SET foo1 bar1");
    EXPECT_EMBEDIS_OK("SET foo bar");

    s = "SET ";
    s += embedis_dictionary_keys[0].name;
    s += " bar2";
    EXPECT_EMBEDIS_OK(s);

    EXPECT_EMBEDIS_OK("SET foo good");
    EXPECT_EMBEDIS_OK("SET foo3 bar3");
    EXPECT_EMBEDIS_STRING("GET foo", "good");
    EXPECT_EMBEDIS_OK("DEL foo");
    EXPECT_EMBEDIS_NULL("GET foo");
    EXPECT_EMBEDIS_STRING("GET foo1", "bar1");

    s = "GET ";
    s += embedis_dictionary_keys[0].name;
    EXPECT_EMBEDIS_STRING(s, "bar2");

    EXPECT_EMBEDIS_STRING("GET foo3", "bar3");

    s = "DEL ";
    s += embedis_dictionary_keys[0].name;
    EXPECT_EMBEDIS_OK(s);

    s = "GET ";
    s += embedis_dictionary_keys[0].name;
    EXPECT_EMBEDIS_NULL(s);
}


TEST(DictRAM, KEYS) {
    embedis_init();
    mock_ram_erase();

    EXPECT_EMBEDIS_OK("SELECT RAM");
    EXPECT_EMBEDIS_ARRAY("KEYS", {});

    EXPECT_EMBEDIS_OK("SET foo bar");
    EXPECT_EMBEDIS_ARRAY("KEYS", {"foo"});

    std::string s;
    s = "SET ";
    s += embedis_dictionary_keys[0].name;
    s += " bar2";
    EXPECT_EMBEDIS_OK(s);

    std::vector<std::string> a;
    a.push_back("foo");
    a.push_back(embedis_dictionary_keys[0].name);
    EXPECT_EMBEDIS_ARRAY("KEYS", a);
}


TEST(DictRAM, ValueOverflow) {
    std::string s1, s2;
    size_t len;

    embedis_init();
    mock_ram_erase();

    EXPECT_EMBEDIS_OK("SELECT RAM");

    // free space for a key of len 3
    len = mock_ram_access.size();
    len -= 2 + 2 + 3 + 2;

    // Just barely fits
    s2.assign(len, 'x');
    s1 = "SET foo ";
    s1 += s2;
    EXPECT_EMBEDIS_OK(s1);
    EXPECT_EMBEDIS_STRING("GET foo", s2);

    // One extra value char causes fail
    s2.assign(len+1, 'x');
    s1 = "SET foo ";
    s1 += s2;
    EXPECT_EMBEDIS_OK("SET foo bar");
    EXPECT_EMBEDIS_ERROR(s1);
    EXPECT_EMBEDIS_STRING("GET foo", "bar");
}


TEST(DictRAM, KeyOverflow) {
    std::string s1, s2;
    size_t len;

    embedis_init();
    mock_ram_erase();

    EXPECT_EMBEDIS_OK("SELECT RAM");

    // free key space for a value of len 3
    len = mock_ram_access.size();
    len -= 2 + 2 + 3 + 2;

    // Just barely fits
    s2.assign(len, 'x');
    s1 = "SET ";
    s1 += s2 + " bar";
    EXPECT_EMBEDIS_OK(s1);
    s1 = "GET ";
    s1 += s2;
    EXPECT_EMBEDIS_STRING(s1, "bar");

    // One extra value char causes fail
    s2.assign(len+1, 'x');
    s1 = "SET foo ";
    s1 += s2 + " bar";
    EXPECT_EMBEDIS_ERROR(s1);
}
