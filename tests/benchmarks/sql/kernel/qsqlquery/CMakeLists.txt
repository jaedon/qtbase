# Generated from qsqlquery.pro.

#####################################################################
## tst_bench_qsqlquery Binary:
#####################################################################

add_qt_benchmark(tst_bench_qsqlquery
    SOURCES
        main.cpp
    PUBLIC_LIBRARIES
        Qt::CorePrivate
        Qt::Sql
        Qt::SqlPrivate
        Qt::Test
)
