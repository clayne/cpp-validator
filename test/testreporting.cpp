#include <boost/test/unit_test.hpp>

#include <dracosha/validator/validator.hpp>
#include <dracosha/validator/adapters/reporting_adapter.hpp>
#include <dracosha/validator/detail/hint_helper.hpp>

using namespace DRACOSHA_VALIDATOR_NAMESPACE;

namespace
{
struct NonCopyable
{
    NonCopyable()=default;
    ~NonCopyable()=default;
    NonCopyable(const NonCopyable&)=delete;
    NonCopyable(NonCopyable&&)=default;
    NonCopyable& operator= (const NonCopyable&)=delete;
    NonCopyable& operator= (NonCopyable&&)=default;

    std::string val;
};
}

BOOST_AUTO_TEST_SUITE(TestReporting)
#if 1
BOOST_AUTO_TEST_CASE(CheckReportingAdapter)
{
    std::string report;
    NonCopyable nc;
    auto ra1=make_reporting_adapter(nc,report);
    using adapter_type1=decltype(ra1);
    static_assert(std::is_same<typename adapter_type1::reporter_type,decltype(make_reporter(report))>::value,"");
    static_assert(!std::is_lvalue_reference<typename adapter_type1::reporter_type>::value,"");
    static_assert(!std::is_rvalue_reference<typename adapter_type1::reporter_type>::value,"");

    auto rp2=make_reporter(report);
    auto& rp_ref2=rp2;
    auto ra2=make_reporting_adapter(nc,rp_ref2);
    using adapter_type2=decltype(ra2);
    static_assert(std::is_same<typename adapter_type2::reporter_type,decltype(rp_ref2)>::value,"");
    static_assert(std::is_lvalue_reference<typename adapter_type2::reporter_type>::value,"");

    auto ra3=make_reporting_adapter(nc,rp2);
    using adapter_type3=decltype(ra3);
    static_assert(std::is_lvalue_reference<typename adapter_type3::reporter_type>::value,"");

    BOOST_CHECK(true);
}

BOOST_AUTO_TEST_CASE(CheckValidationReport)
{
    std::map<std::string,size_t> m1={{"field1",1}};

    auto v1=validator(
                _["field1"](gte,10)
            );

    std::string rep1;
    auto ra1=make_reporting_adapter(m1,rep1);
    BOOST_CHECK(!v1.apply(ra1));
    BOOST_CHECK_EQUAL(rep1,std::string("field1 must be greater than or equal to 10"));
    rep1.clear();

    auto v2=validator(
                _["field1"](length(gte,10))
            );
    BOOST_CHECK(!v2.apply(ra1));
    BOOST_CHECK_EQUAL(rep1,std::string("length of field1 must be greater than or equal to 10"));
    rep1.clear();

    std::vector<size_t> vec1={10,20,30,40,50};
    auto ra2=make_reporting_adapter(vec1,rep1);
    auto v3=validator(
                _[1](gte,100)
            );
    BOOST_CHECK(!v3.apply(ra2));
    BOOST_CHECK_EQUAL(rep1,std::string("element #1 must be greater than or equal to 100"));
    rep1.clear();
}

BOOST_AUTO_TEST_CASE(CheckValidationExistsReport)
{
    std::map<std::string,size_t> m1={{"field1",1}};
    std::string rep1;
    auto ra1=make_reporting_adapter(m1,rep1);

    auto v1=validator(
                _["field2"](exists,true)
            );
    BOOST_CHECK(!v1.apply(ra1));
    BOOST_CHECK_EQUAL(rep1,"field2 must exist");
    rep1.clear();

    auto v2=validator(
                _["field1"](exists,false)
            );
    BOOST_CHECK(!v2.apply(ra1));
    BOOST_CHECK_EQUAL(rep1,"field1 must not exist");
    rep1.clear();

    auto v3=validator(
                _["field2"](exists,false)
            );
    BOOST_CHECK(v3.apply(ra1));
    rep1.clear();

    //! @todo Propagate operator to check existence
    auto v4=validator(
                _["field2"](_(exists,"should exist"),true)
            );
    BOOST_CHECK(!v4.apply(ra1));
    BOOST_CHECK_EQUAL(rep1,"field2 should exist");
    rep1.clear();
}

BOOST_AUTO_TEST_CASE(CheckValidationReportAggregation)
{
    std::map<std::string,std::string> m1={{"field1","value1"}};
    std::string rep1;
    auto ra1=make_reporting_adapter(m1,rep1);

    auto v1=validator(
                _["field1"](value(gte,"z1000") ^OR^ length(lt,3))
            );
    BOOST_CHECK(!v1.apply(ra1));
    BOOST_CHECK_EQUAL(rep1,std::string("field1 must be greater than or equal to z1000 OR length of field1 must be less than 3"));
    rep1.clear();

    auto v2=validator(
                _["field1"](gte,"v"),
                _["field1"](size(gte,100))
            );
    BOOST_CHECK(!v2.apply(ra1));
    BOOST_CHECK_EQUAL(rep1,std::string("size of field1 must be greater than or equal to 100"));
    rep1.clear();

    auto v3=validator(
                _["field1"](gte,"xxxxxx")
                 ^OR^
                _["field1"](size(gte,100) ^OR^ value(gte,"zzzzzzzzzzzz"))
            );
    BOOST_CHECK(!v3.apply(ra1));
    BOOST_CHECK_EQUAL(rep1,std::string("field1 must be greater than or equal to xxxxxx OR size of field1 must be greater than or equal to 100 OR field1 must be greater than or equal to zzzzzzzzzzzz"));
    rep1.clear();

    auto v4=validator(
                _["field1"](gte,"xxxxxx")
                 ^OR^
                _["field1"](size(gte,100) ^AND^ value(gte,"zzzzzzzzzzzz"))
            );
    BOOST_CHECK(!v4.apply(ra1));
    BOOST_CHECK_EQUAL(rep1,std::string("field1 must be greater than or equal to xxxxxx OR size of field1 must be greater than or equal to 100"));
    rep1.clear();

    auto v5=validator(
                _["field1"](gte,"xxxxxx")
                 ^OR^
                _["field1"](size(gte,1) ^AND^ value(gte,"zzzzzzzzzzzz"))
            );
    BOOST_CHECK(!v5.apply(ra1));
    BOOST_CHECK_EQUAL(rep1,std::string("field1 must be greater than or equal to xxxxxx OR field1 must be greater than or equal to zzzzzzzzzzzz"));
    rep1.clear();
}

BOOST_AUTO_TEST_CASE(CheckValidationReportNot)
{
    std::map<std::string,size_t> m1={{"field1",10}};
    std::string rep1;
    auto ra1=make_reporting_adapter(m1,rep1);

    auto v1=validator(
                NOT(_["field1"](eq,10))
            );
    BOOST_CHECK(!v1.apply(ra1));
    BOOST_CHECK_EQUAL(rep1,std::string("NOT field1 must be equal to 10"));
    rep1.clear();

    auto v2=validator(
                _["field1"](NOT(value(gte,1)))
            );
    BOOST_CHECK(!v2.apply(ra1));
    BOOST_CHECK_EQUAL(rep1,std::string("NOT field1 must be greater than or equal to 1"));
    rep1.clear();

    auto v3=validator(
                NOT(
                    _["field1"](eq,10),
                    _["field1"](gte,8)
                )
            );
    BOOST_CHECK(!v3.apply(ra1));
    BOOST_CHECK_EQUAL(rep1,std::string("NOT (field1 must be equal to 10 AND field1 must be greater than or equal to 8)"));
    rep1.clear();

    // validator reports only the first matched condition for nested OR operator
    auto v4=validator(
                NOT(
                    _["field1"](eq,10)
                    ^OR^
                    _["field1"](gte,100)
                )
            );
    BOOST_CHECK(!v4.apply(ra1));
    BOOST_CHECK_EQUAL(rep1,std::string("NOT field1 must be equal to 10"));
    rep1.clear();
}

BOOST_AUTO_TEST_CASE(CheckNestedValidationReport)
{
    std::map<std::string,std::map<size_t,size_t>> m1={
            {"field1",{{1,10}}}
        };
    std::string rep1;
    auto ra1=make_reporting_adapter(m1,rep1);

    auto v1=validator(
                _[size](gte,0),
                _["field1"][1](gte,10)
            );
    BOOST_CHECK(v1.apply(ra1));
    rep1.clear();

    auto v2=validator(
                _["field1"][1](lt,5)
            );
    BOOST_CHECK(!v2.apply(ra1));
    BOOST_CHECK_EQUAL(rep1,"element #1 of field1 must be less than 5");
    rep1.clear();

    auto v3=validator(
                _[100](lt,5)
            );
    BOOST_CHECK(v3.apply(ra1));
    rep1.clear();
}

BOOST_AUTO_TEST_CASE(CheckOtherFieldReport)
{
    std::map<std::string,int> m1={
            {"field1",10},
            {"field2",5},
            {"field3",5}
        };
    std::string rep1;
    auto ra1=make_reporting_adapter(m1,rep1);

    auto v1=validator(
                _["field1"](gt,_["field2"]),
                _["field2"](eq,_["field3"])
            );
    BOOST_CHECK(v1.apply(ra1));
    rep1.clear();

    auto v2=validator(
                _["field2"](gt,_["field3"]) ^OR^ _["field3"](eq,_["field1"])
            );
    BOOST_CHECK(!v2.apply(ra1));
    BOOST_CHECK_EQUAL(rep1,"field2 must be greater than field3 OR field3 must be equal to field1");
    rep1.clear();
}

BOOST_AUTO_TEST_CASE(CheckSampleObjectReport)
{
    std::map<std::string,int> m1={
            {"field1",10},
            {"field2",5},
            {"field3",5}
        };
    std::map<std::string,int> m2={
            {"field1",10},
            {"field2",5},
            {"field3",5}
        };
    std::string rep1;
    auto ra1=make_reporting_adapter(m1,rep1);

    auto v1=validator(
                _["field1"](gte,_(m2)),
                _["field2"](eq,_(m2))
            );
    BOOST_CHECK(v1.apply(ra1));
    rep1.clear();

    auto v2=validator(
                _["field2"](gt,_(m2)) ^OR^ _["field3"](lt,_(m2))
            );
    BOOST_CHECK(!v2.apply(ra1));
    BOOST_CHECK_EQUAL(rep1,"field2 must be greater than field2 of sample OR field3 must be less than field3 of sample");
    rep1.clear();

    auto v3=validator(
                _["field2"](gt,_(_(m2),"m2")) ^OR^ _["field3"](lt,_(_(m2),"m2"))
            );
    BOOST_CHECK(!v3.apply(ra1));
    BOOST_CHECK_EQUAL(rep1,"field2 must be greater than field2 of m2 OR field3 must be less than field3 of m2");
    rep1.clear();

    std::map<size_t,size_t> m3={
        {1,10},
        {2,20},
        {3,30}
    };
    std::map<size_t,size_t> m4={
        {1,5},
        {2,20}
    };
    auto ra2=make_reporting_adapter(m4,rep1);
    auto v4=validator(
                _[1](gt,_(_(m3),"m3"))
            );
    BOOST_CHECK(!v4.apply(ra2));
    BOOST_CHECK_EQUAL(rep1,"element #1 must be greater than element #1 of m3");
    rep1.clear();

    std::map<std::string,size_t> m5={
        {"one",5},
        {"two",20}
    };
    auto v5=validator(
                _[1](gt,_(m5))
            );
    BOOST_CHECK(v5.apply(ra2));

    auto ra5=make_reporting_adapter(m5,rep1);
    BOOST_CHECK(v5.apply(ra5));

    auto v6=validator(
                _["three"](gt,_(m5))
            );
    BOOST_CHECK(v6.apply(ra5));
}
#endif
BOOST_AUTO_TEST_CASE(CheckAggregationAnyReport)
{
    std::string rep1;

#if 1
    std::map<std::string,std::map<size_t,std::string>> m1={
            {"field1",
             {
                 {1,"value1"},
                 {2,"value2"},
                 {3,"value3"},
                 {4,"value4"}
             }
            }
        };
    auto ra1=make_reporting_adapter(m1,rep1);

    auto v1=validator(
                _["field1"](ANY(value(gte,"zzz")))
            );
    BOOST_CHECK(!v1.apply(ra1));
    BOOST_CHECK_EQUAL(rep1,"at least one element of field1 must be greater than or equal to zzz");
    rep1.clear();

    auto v2=validator(
                _["field1"](ANY(value(gte,"zzz") ^AND^ size(eq,100)))
            );
    BOOST_CHECK(!v2.apply(ra1));
    BOOST_CHECK_EQUAL(rep1,"at least one element of field1 must be greater than or equal to zzz");
    rep1.clear();

    auto v3=validator(
                _["field1"](ANY(value(gte,"zzz") ^OR^ size(eq,100)))
            );
    BOOST_CHECK(!v3.apply(ra1));
    BOOST_CHECK_EQUAL(rep1,"at least one element of field1 must be greater than or equal to zzz OR size of at least one element of field1 must be equal to 100");
    rep1.clear();

    std::map<size_t,std::string> m2={
        {1,"value1"},
        {2,"value2"},
        {3,"value3"},
        {4,"value4"}
    };
    auto ra2=make_reporting_adapter(m2,rep1);

    auto v4=validator(
                ANY(value(gte,"zzz") ^AND^ size(eq,100))
            );
    BOOST_CHECK(!v4.apply(ra2));
    BOOST_CHECK_EQUAL(rep1,"at least one element must be greater than or equal to zzz");
    rep1.clear();

    auto v5=validator(
                ANY(value(gte,"zzz") ^OR^ size(eq,100))
            );
    BOOST_CHECK(!v5.apply(ra2));
    BOOST_CHECK_EQUAL(rep1,"at least one element must be greater than or equal to zzz OR size of at least one element must be equal to 100");
    rep1.clear();
#endif
    std::map<size_t,std::vector<size_t>> m3={
        {1,{1,2,3,4,5}},
        {10,{10,20,30,40,50}},
        {100,{100,200,300,400,500}}
    };
    auto ra3=make_reporting_adapter(m3,rep1);
    auto v6=validator(
                ANY(size(gte,5) ^AND^ ANY(value(gte,1)))
            );
    BOOST_CHECK(v6.apply(ra3));
    rep1.clear();
#if 1
    auto v7=validator(
                ANY(size(gte,5) ^AND^ ANY(value(gte,1000)))
            );
    BOOST_CHECK(!v7.apply(ra3));
    BOOST_CHECK_EQUAL(rep1,"at least one element of at least one element must be greater than or equal to 1000");
    rep1.clear();

    std::map<size_t,std::map<size_t,std::vector<size_t>>> m4={
        {5,
         {{10,{10,20,30,40,50}}}
        }
    };
    auto ra4=make_reporting_adapter(m4,rep1);
    auto v8=validator(
                _[5](ANY(size(gte,1) ^AND^ ANY(value(gte,1000))))
            );
    BOOST_CHECK(!v8.apply(ra4));
    BOOST_CHECK_EQUAL(rep1,"at least one element of at least one element of element #5 must be greater than or equal to 1000");
    rep1.clear();
#endif
}
#if 1
BOOST_AUTO_TEST_CASE(CheckAggregationAllReport)
{
    std::map<std::string,std::map<size_t,std::string>> m1={
            {"field1",
             {
                 {1,"value1"},
                 {2,"value2"},
                 {3,"value3"},
                 {4,"value4"}
             }
            }
        };

    std::string rep1;
    auto ra1=make_reporting_adapter(m1,rep1);

    auto v1=validator(
                _["field1"](ALL(value(gte,"value3")))
            );
    BOOST_CHECK(!v1.apply(ra1));
    BOOST_CHECK_EQUAL(rep1,"each element of field1 must be greater than or equal to value3");
    rep1.clear();

    auto v2=validator(
                _["field1"](ALL(value(gte,"val") ^AND^ size(gte,100)))
            );
    BOOST_CHECK(!v2.apply(ra1));
    BOOST_CHECK_EQUAL(rep1,"size of each element of field1 must be greater than or equal to 100");
    rep1.clear();

    auto v3=validator(
                _["field1"](ALL(value(gte,"zzz") ^OR^ size(eq,100)))
            );
    BOOST_CHECK(!v3.apply(ra1));
    BOOST_CHECK_EQUAL(rep1,"each element of field1 must be greater than or equal to zzz OR size of each element of field1 must be equal to 100");
    rep1.clear();

    std::map<size_t,std::string> m2={
        {1,"value1"},
        {2,"value2"},
        {3,"value3"},
        {4,"value444"}
    };
    auto ra2=make_reporting_adapter(m2,rep1);

    auto v4=validator(
                ALL(size(gte,1) ^AND^ value(lte,"value3"))
            );
    BOOST_CHECK(!v4.apply(ra2));
    BOOST_CHECK_EQUAL(rep1,"each element must be less than or equal to value3");
    rep1.clear();

    auto v5=validator(
                ALL(value(gte,"value5") ^OR^ size(lte,7))
            );
    BOOST_CHECK(!v5.apply(ra2));
    BOOST_CHECK_EQUAL(rep1,"each element must be greater than or equal to value5 OR size of each element must be less than or equal to 7");
    rep1.clear();

    std::map<size_t,std::vector<size_t>> m3={
        {1,{1,2,3,4,5}},
        {10,{10,20,30,40,50}},
        {100,{100,200,300,400,500}}
    };
    auto ra3=make_reporting_adapter(m3,rep1);
    auto v6=validator(
                ALL(size(gte,5) ^AND^ ALL(value(gte,1)))
            );
    BOOST_CHECK(v6.apply(ra3));
    rep1.clear();

    auto v7=validator(
                ALL(size(gte,5) ^AND^ ALL(value(lt,300)))
            );
    BOOST_CHECK(!v7.apply(ra3));
    BOOST_CHECK_EQUAL(rep1,"each element of each element must be less than 300");
    rep1.clear();

    std::map<size_t,std::map<size_t,std::vector<size_t>>> m4={
        {5,
         {{10,{10,20,30,40,50}}}
        }
    };
    auto ra4=make_reporting_adapter(m4,rep1);
    auto v8=validator(
                _[5](ALL(size(gte,1) ^AND^ ALL(value(lt,50))))
            );
    BOOST_CHECK(!v8.apply(ra4));
    BOOST_CHECK_EQUAL(rep1,"each element of each element of element #5 must be less than 50");
    rep1.clear();
}

BOOST_AUTO_TEST_CASE(CheckFlagValidationReport)
{
    std::string rep1;

    std::map<std::string,size_t> m1={{"field1",1}};
    auto ra1=make_reporting_adapter(m1,rep1);

    auto v1=validator(
                empty(flag,true)
            );
    BOOST_CHECK(!v1.apply(ra1));
    BOOST_CHECK_EQUAL(rep1,"must be empty");
    rep1.clear();

    auto v2=validator(
                empty(flag,false)
            );
    BOOST_CHECK(v2.apply(ra1));
    rep1.clear();

    auto v3=validator(
                empty(flag(flag_on_off),true)
            );
    BOOST_CHECK(!v3.apply(ra1));
    BOOST_CHECK_EQUAL(rep1,"empty must be on");
    rep1.clear();

    std::map<std::string,std::map<size_t,size_t>> m2={{"field1",{{1,10}}}};
    auto ra2=make_reporting_adapter(m2,rep1);

    auto v4=validator(
                _["field1"](empty(flag,true))
            );
    BOOST_CHECK(!v4.apply(ra2));
    BOOST_CHECK_EQUAL(rep1,"field1 must be empty");
    rep1.clear();

    auto v5=validator(
                _["field1"](empty(flag(flag_checked_unchecked),true))
            );
    BOOST_CHECK(!v5.apply(ra2));
    BOOST_CHECK_EQUAL(rep1,"empty of field1 must be checked");
    rep1.clear();

    auto v5_1=validator(
                _["field1"](empty(_(flag,flag_checked_unchecked),true))
            );
    BOOST_CHECK(!v5_1.apply(ra2));
    BOOST_CHECK_EQUAL(rep1,"empty of field1 must be checked");
    rep1.clear();

    auto v6=validator(
                _["field1"](empty(flag("expected to be true"),true))
            );
    BOOST_CHECK(!v6.apply(ra2));
    BOOST_CHECK_EQUAL(rep1,"empty of field1 expected to be true");
    rep1.clear();

    auto v6_1=validator(
                _["field1"](empty(_(flag,"expected to be true"),true))
            );
    BOOST_CHECK(!v6_1.apply(ra2));
    BOOST_CHECK_EQUAL(rep1,"empty of field1 expected to be true");
    rep1.clear();

    auto v7=validator(
                _[empty](flag,true)
            );
    BOOST_CHECK(!v7.apply(ra1));
    BOOST_CHECK_EQUAL(rep1,"must be empty");
    rep1.clear();

    auto v8=validator(
                _["field1"][empty](flag,true)
            );
    BOOST_CHECK(!v8.apply(ra2));
    BOOST_CHECK_EQUAL(rep1,"field1 must be empty");
    rep1.clear();

    auto v9=validator(
                _["field1"][empty](flag(flag_set_unset),true)
            );
    BOOST_CHECK(!v9.apply(ra2));
    BOOST_CHECK_EQUAL(rep1,"empty of field1 must be set");
    rep1.clear();

    auto v9_1=validator(
                _["field1"][empty](_(flag,flag_set_unset),true)
            );
    BOOST_CHECK(!v9_1.apply(ra2));
    BOOST_CHECK_EQUAL(rep1,"empty of field1 must be set");
    rep1.clear();

    auto v10=validator(
                _["field1"][empty](flag("expected to be true"),true)
            );
    BOOST_CHECK(!v10.apply(ra2));
    BOOST_CHECK_EQUAL(rep1,"empty of field1 expected to be true");
    rep1.clear();

    auto v11=validator(
                _["field1"][empty](_(flag,"expected to be true"),true)
            );
    BOOST_CHECK(!v11.apply(ra2));
    BOOST_CHECK_EQUAL(rep1,"empty of field1 expected to be true");
    rep1.clear();
}

BOOST_AUTO_TEST_CASE(CheckContainsValidationReport)
{
    std::string rep1;

    std::map<std::string,size_t> m1={{"field1",1}};

    auto v1=validator(
                contains,"field2"
            );
    auto ra1=make_reporting_adapter(m1,rep1);
    BOOST_CHECK(!v1.apply(ra1));
    BOOST_CHECK_EQUAL(rep1,"must contain field2");
    rep1.clear();

    std::map<std::string,std::map<size_t,std::string>> m2={
            {"field1",
             {
                 {1,"value10"},{2,"value100"},{3,"hi"},{4,"hello"}
             }
            }
        };
    auto ra2=make_reporting_adapter(m2,rep1);
    auto v2=validator(
                _["field1"](contains,30)
            );
    BOOST_CHECK(!v2.apply(ra2));
    BOOST_CHECK_EQUAL(rep1,std::string("field1 must contain element #30"));
    rep1.clear();
}
#endif
BOOST_AUTO_TEST_SUITE_END()
