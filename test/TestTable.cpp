#include "UnitTest.h"
#include "Table.h"
#include "String.h"

TEST_CASE(table1)
{
    luna::Table t;

    for (int i = 0; i < 3; ++i)
    {
        luna::Value value;
        value.num_ = i + 1;
        value.type_ = luna::ValueT_Number;
        EXPECT_TRUE(t.SetArrayValue(i + 1, value));
    }

    luna::Value key;
    luna::Value value;
    EXPECT_TRUE(t.FirstKeyValue(key, value));
    EXPECT_TRUE(key.type_ == luna::ValueT_Number);
    EXPECT_TRUE(key.num_ == static_cast<double>(1));
    EXPECT_TRUE(value.type_ == luna::ValueT_Number);
    EXPECT_TRUE(value.num_ == static_cast<double>(1));

    for (int i = 1; i < 3; ++i)
    {
        luna::Value next_key;
        luna::Value next_value;
        EXPECT_TRUE(t.NextKeyValue(key, next_key, next_value));
        EXPECT_TRUE(next_key.type_ == luna::ValueT_Number);
        EXPECT_TRUE(next_key.num_ == static_cast<double>(i + 1));
        EXPECT_TRUE(next_value.type_ == luna::ValueT_Number);
        EXPECT_TRUE(next_value.num_ == static_cast<double>(i + 1));
        key = next_key;
    }

    EXPECT_TRUE(!t.NextKeyValue(key, key, value));

    value = t.GetValue(key);
    EXPECT_TRUE(value.type_ == luna::ValueT_Number);
    EXPECT_TRUE(value.num_ == static_cast<double>(3));
}

TEST_CASE(table2)
{
    luna::Table t;
    luna::String key_str("key");
    luna::String value_str("value");

    luna::Value key;
    luna::Value value;

    key.type_ = luna::ValueT_Obj;
    key.obj_ = &key_str;
    value.type_ = luna::ValueT_Obj;
    value.obj_ = &value_str;

    t.SetValue(key, value);
    value = t.GetValue(key);

    EXPECT_TRUE(value.type_ == luna::ValueT_Obj);
    EXPECT_TRUE(value.obj_ == &value_str);

    luna::Value key_not_existed;
    key_not_existed.type_ = luna::ValueT_Obj;
    key_not_existed.obj_ = &value_str;

    value = t.GetValue(key_not_existed);
    EXPECT_TRUE(value.type_ == luna::ValueT_Nil);

    EXPECT_TRUE(t.FirstKeyValue(key, value));
    EXPECT_TRUE(key.obj_ == &key_str);
    EXPECT_TRUE(value.obj_ == &value_str);

    EXPECT_TRUE(!t.NextKeyValue(key, key, value));
}

TEST_CASE(table3)
{
    luna::Table t;
    luna::Value key;
    luna::Value value;

    key.type_ = luna::ValueT_Bool;
    key.bvalue_ = true;

    value.type_ = luna::ValueT_Bool;
    value.bvalue_ = false;

    t.SetValue(key, value);
    value = t.GetValue(key);
    EXPECT_TRUE(value.type_ == luna::ValueT_Bool);
    EXPECT_TRUE(value.bvalue_ == false);

    t.SetValue(value, key);
    key = t.GetValue(value);
    EXPECT_TRUE(key.type_ == luna::ValueT_Bool);
    EXPECT_TRUE(key.bvalue_ == true);

    luna::Value nil;
    value = t.GetValue(nil);
    EXPECT_TRUE(value.type_ == luna::ValueT_Nil);
}
