/**
 * This file is part of the MobiusCore project.
 * See AUTHORS file for copyright information.
 */

#ifndef _PREPAREDSTATEMENT_H
#define _PREPAREDSTATEMENT_H

#include "Define.h"
#include "SQLOperation.h"
#include <future>
#include <vector>

#ifdef __APPLE__
#undef TYPE_BOOL
#endif

//- Union for data buffer (upper-level bind -> queue -> lower-level bind)
union PreparedStatementDataUnion
{
    bool boolean;
    uint8 ui8;
    int8 i8;
    uint16 ui16;
    int16 i16;
    uint32 ui32;
    int32 i32;
    uint64 ui64;
    int64 i64;
    float f;
    double d;
};

//- This enum helps us differ data held in above union
enum PreparedStatementValueType
{
    TYPE_BOOL,
    TYPE_UI8,
    TYPE_UI16,
    TYPE_UI32,
    TYPE_UI64,
    TYPE_I8,
    TYPE_I16,
    TYPE_I32,
    TYPE_I64,
    TYPE_FLOAT,
    TYPE_DOUBLE,
    TYPE_STRING,
    TYPE_BINARY,
    TYPE_NULL
};

struct PreparedStatementData
{
    PreparedStatementDataUnion data;
    PreparedStatementValueType type;
    std::vector<uint8> binary;
};

//- Forward declare
class MySQLPreparedStatement;

//- Upper-level class that is used in code
class DATABASE_API PreparedStatementBase
{
    friend class PreparedStatementTask;
    friend class MySQLPreparedStatement;
    friend class MySQLConnection;

    public:
        explicit PreparedStatementBase(uint32 index, uint8 capacity);
        virtual ~PreparedStatementBase();

        void setBool(const uint8 index, const bool value);
        void setUInt8(const uint8 index, const uint8 value);
        void setUInt16(const uint8 index, const uint16 value);
        void setUInt32(const uint8 index, const uint32 value);
        void setUInt64(const uint8 index, const uint64 value);
        void setInt8(const uint8 index, const int8 value);
        void setInt16(const uint8 index, const int16 value);
        void setInt32(const uint8 index, const int32 value);
        void setInt64(const uint8 index, const int64 value);
        void setFloat(const uint8 index, const float value);
        void setDouble(const uint8 index, const double value);
        void setString(const uint8 index, const std::string& value);
        void setBinary(const uint8 index, const std::vector<uint8>& value);
        void setNull(const uint8 index);

        uint32 GetIndex() const { return m_index; }

    protected:
        void BindParameters(MySQLPreparedStatement* stmt);

    protected:
        MySQLPreparedStatement* m_stmt;
        uint32 m_index;

        //- Buffer of parameters, not tied to MySQL in any way yet
        std::vector<PreparedStatementData> statement_data;

        PreparedStatementBase(PreparedStatementBase const& right) = delete;
        PreparedStatementBase& operator=(PreparedStatementBase const& right) = delete;
};

template<typename T>
class PreparedStatement : public PreparedStatementBase
{
public:
    explicit PreparedStatement(uint32 index, uint8 capacity) : PreparedStatementBase(index, capacity)
    {
    }

private:
    PreparedStatement(PreparedStatement const& right) = delete;
    PreparedStatement& operator=(PreparedStatement const& right) = delete;
};

//- Lower-level class, enqueuable operation
class DATABASE_API PreparedStatementTask : public SQLOperation
{
    public:
        PreparedStatementTask(PreparedStatementBase* stmt, bool async = false);
        ~PreparedStatementTask();

        bool Execute() override;
        PreparedQueryResultFuture GetFuture() { return m_result->get_future(); }

    protected:
        PreparedStatementBase* m_stmt;
        bool m_has_result;
        PreparedQueryResultPromise* m_result;
};
#endif
