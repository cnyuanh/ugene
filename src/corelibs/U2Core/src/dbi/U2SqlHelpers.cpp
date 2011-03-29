/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include "U2SqlHelpers.h"

#include <sqlite3.h>

namespace U2 {



void SQLiteUtils::addLimit(QString& sql, qint64 offset, qint64 count) {
    if (count == -1) {
        return;
    }
    sql = sql + QString(" LIMIT %1, %2").arg(offset).arg(count).toAscii();
}

#define DB_ID_OFFSET    0
#define TYPE_OFFSET     8
#define DB_EXTRA_OFFSET 10
#define DATAID_MIN_LEN  10

static QByteArray emptyId;
U2DataId SQLiteUtils::toU2DataId(qint64 id, U2DataType type, const QByteArray& dbExtra) {
    if (id == 0) {
        return emptyId;
    }
    assert(sizeof(U2DataType)==2);
    int extraLen = dbExtra.size();
    int len = DATAID_MIN_LEN + extraLen;
#if QT_VERSION >= QT_VERSION_CHECK(4, 7, 0)
    QByteArray res(len, Qt::Uninitialized);
#else
    QByteArray res(len, (char)0);
#endif
    char* data = res.data();
    ((qint64*)(data + DB_ID_OFFSET))[0] = id;
    ((U2DataType*)(data + TYPE_OFFSET))[0] = type;
    if (extraLen > 0) {
        qMemCopy(data + DB_EXTRA_OFFSET, dbExtra.constData(), dbExtra.size());
    }
    return res;
}

quint64 SQLiteUtils::toDbiId(const U2DataId& id) {
    if (id.size() < DATAID_MIN_LEN) {
        return 0;
    }
    return *(qint64*)(id.constData() + DB_ID_OFFSET);
}

U2DataType SQLiteUtils::toType(const U2DataId& id) {
    if (id.size() < DATAID_MIN_LEN) {
        return 0;
    }
    return *(qint64*)(id.constData() + TYPE_OFFSET);
}

QByteArray SQLiteUtils::toDbExtra(const U2DataId& id) {
    if (id.size() < DATAID_MIN_LEN) {
        return emptyId;
    }
    return QByteArray(id.constData() + DB_EXTRA_OFFSET);
}

qint64 SQLiteUtils::remove(const QString& table, const QString& field, const U2DataId& id, qint64 expectedRows, DbRef* db, U2OpStatus& os) {
    SQLiteQuery q(QString("DELETE FROM %1 WHERE %2 = ?1").arg(table).arg(field), db, os);
    q.bindDataId(1, id);
    return q.update(expectedRows);
}


QString SQLiteUtils::text(const U2DataId& id) {
    QString res = QString("[Id: %1, Type: %2, Extra: %3]").arg(toDbiId(id)).arg(int(toType(id))).arg(toDbExtra(id).constData());
    return res;
}

//////////////////////////////////////////////////////////////////////////
// L10N
QString SQLiteL10n::queryError(const QString& err) {
    return tr("Error querying database: %1").arg(err);
}

QString SQLiteL10n::tooManyResults() {
    return tr("Found more results than expected!");
}

//////////////////////////////////////////////////////////////////////////
// Query


SQLiteQuery::SQLiteQuery(const QString& _sql, DbRef* d, U2OpStatus& _os) 
: db(d), os(_os), st(NULL), sql(_sql)
{
    prepare();
}

SQLiteQuery::SQLiteQuery(const QString& _sql, qint64 offset, qint64 count, DbRef* d, U2OpStatus& _os)
: db(d), os(_os), st(NULL), sql(_sql)
{
    SQLiteUtils::addLimit(sql, offset, count);
    prepare();
}

void SQLiteQuery::prepare() {
    if (os.hasError()) {
        return;
    }
    QByteArray utf8 = sql.toUtf8();
    int rc = sqlite3_prepare_v2(db->handle, utf8.constData() ,utf8.size(), &st, NULL);
    if (rc != SQLITE_OK) {
        setError(SQLiteL10n::queryError(sqlite3_errmsg(db->handle)));
        return;
    }
    assert(st!=NULL);
}

SQLiteQuery::~SQLiteQuery() {
    if (st != NULL) {
        int rc = sqlite3_finalize(st);
        if (rc != SQLITE_OK && !os.hasError()) {
            setError(SQLiteL10n::queryError(sqlite3_errmsg(db->handle)));
            return;
        }
    }
}



bool SQLiteQuery::reset(bool clearBindings) {
    if (hasError()) {
        return false;
    }
    assert(st != NULL);
    if (clearBindings) {
        int rc = sqlite3_clear_bindings(st);
        if (rc != SQLITE_OK) {
            setError(SQLiteL10n::queryError(sqlite3_errmsg(db->handle)));
            return false;
        }
    }
    int rc = sqlite3_reset(st);
    if (rc != SQLITE_OK) {
        setError(SQLiteL10n::queryError(sqlite3_errmsg(db->handle)));
        return false;
    }
    return true;
}

bool SQLiteQuery::step() {
    if (hasError()) {
        return false;
    }
    assert(st != NULL);
    
    int rc = sqlite3_step(st);
    if (rc == SQLITE_DONE) {
        return false; 
    } else if (rc == SQLITE_ROW) {
        return true;
    }
    setError(SQLiteL10n::tr("Unexpected query result code: %1").arg(rc));
    return false;
}

void SQLiteQuery::ensureDone() {
    bool done = !step();
    if (!done && !hasError()) {
        setError(SQLiteL10n::tooManyResults());
    }
}

int SQLiteQuery::getInt32(int column) const {
    if (hasError()) {
        return -1;
    }
    assert(st!=NULL);
    return sqlite3_column_int(st, column);
}

qint64 SQLiteQuery::getInt64(int column) const {
    if (hasError()) {
        return -1;
    }
    assert(st!=NULL);
    return sqlite3_column_int64(st, column);
}

double SQLiteQuery::getDouble(int column) const {
    if (hasError()) {
        return -1;
    }
    assert(st!=NULL);
    return sqlite3_column_double(st, column);
}

U2DataId SQLiteQuery::getDataId(int column, U2DataType type, const QByteArray& dbExtra) const {
    if (hasError()) {
        return 0;
    }
    assert(st!=NULL);
    U2DataId res = SQLiteUtils::toU2DataId(getInt64(column), type, dbExtra);
    return res;
}


U2DataType SQLiteQuery::getDataType(int column) const {
    if (hasError()) {
        return U2Type::Unknown;
    }
    assert(st!=NULL);
    return (U2DataType)sqlite3_column_int(st, column);
}

QString SQLiteQuery::getString(int column) const {
    if (hasError()) {
        return QString();
    }
    assert(st!=NULL);
    return QString::fromUtf8((const char*)sqlite3_column_text(st, column));
}

QByteArray SQLiteQuery::getCString(int column) const {
    if (hasError()) {
        return emptyId;
    }
    assert(st!=NULL);
    return QByteArray((const char*)sqlite3_column_text(st, column));
}

QByteArray SQLiteQuery::getBlob(int column) const {
    if (hasError()) {
        return emptyId;
    }
    assert(st!=NULL);
    QByteArray res(static_cast<const char *>(sqlite3_column_blob(st, column)), sqlite3_column_bytes(st, column));
    return res;
}


// param binding methods
void SQLiteQuery::bindDataId(int idx, const U2DataId& val) {
    bindInt64(idx, SQLiteUtils::toDbiId(val));
}

void SQLiteQuery::bindType(int idx, U2DataType type) {
    bindInt64(idx, type);
}

void SQLiteQuery::bindText(int idx, const QString& val) {
    if (hasError()) {
        return;
    }
    assert(st!=NULL);
    QByteArray utf8 = val.toUtf8();
    bool transient = true;
    int rc = sqlite3_bind_text(st, idx, utf8, utf8.length(), transient ? SQLITE_TRANSIENT : SQLITE_STATIC);    
    if (rc != SQLITE_OK) {
        setError(SQLiteL10n::tr("Error binding text value! Query: '%1', idx: %2, value: '%3'").arg(sql).arg(idx).arg(val));
        return;
    }
}

void SQLiteQuery::bindInt32(int idx, qint32 val) {
    if (hasError()) {
        return;
    }
    assert(st!=NULL);
    int rc = sqlite3_bind_int(st, idx, val);    
    if (rc != SQLITE_OK) {
        setError(SQLiteL10n::tr("Error binding int32 value! Query: '%1', idx: %2, value: %3").arg(sql).arg(idx).arg(val));
        return;
    }
}

void SQLiteQuery::bindDouble(int idx, double val) {
    if (hasError()) {
        return;
    }
    assert(st!=NULL);
    int rc = sqlite3_bind_double(st, idx, val);    
    if (rc != SQLITE_OK) {
        setError(SQLiteL10n::tr("Error binding int64 value! Query: '%1', idx: %2, value: %3").arg(sql).arg(idx).arg(val));
        return;
    }
}

void SQLiteQuery::bindInt64(int idx, qint64 val) {
    if (hasError()) {
        return;
    }
    assert(st!=NULL);
    int rc = sqlite3_bind_int64(st, idx, val);    
    if (rc != SQLITE_OK) {
        setError(SQLiteL10n::tr("Error binding int64 value! Query: '%1', idx: %2, value: %3").arg(sql).arg(idx).arg(val));
        return;
    }
}

void SQLiteQuery::bindBool(int idx, bool val) {
    if (hasError()) {
        return;
    }
    assert(st!=NULL);
    int b = (val == 0 ? 0 : 1);
    int rc = sqlite3_bind_int(st, idx, b);    
    if (rc != SQLITE_OK) {
        setError(SQLiteL10n::tr("Error binding boolean value! Query: '%1', idx: %2, value: %3").arg(sql).arg(idx).arg(b));
        return;
    }
}

void SQLiteQuery::bindBlob(int idx, const QByteArray& blob, bool transient) {
    if (hasError()) {
        return;
    }
    assert(st!=NULL);
    int rc = SQLITE_OK;
    if (blob.isEmpty()) {
        rc = sqlite3_bind_zeroblob(st, idx, -1);
    } else {
        rc = sqlite3_bind_blob(st, idx, blob.constData(), blob.size(), transient ? SQLITE_TRANSIENT : SQLITE_STATIC);
    }
    if (rc != SQLITE_OK) {
        setError(SQLiteL10n::tr("Error binding blob value! Query: '%1', idx: %2, size: %3").arg(sql).arg(idx).arg(blob.size()));
        return;
    }
}

void SQLiteQuery::execute() {
    update(-1);
}

qint64 SQLiteQuery::update(qint64 expectedRows) {
    if (step()) {
        qint64 res = getInt64(0);
        if (expectedRows != -1 && expectedRows != res) {
            setError(SQLiteL10n::tr("Unexpected row count! Query: '%1', rows: %2").arg(sql).arg(res));
        }
        return res;
    }
    return -1;
}

qint64 SQLiteQuery::insert() {
    QMutexLocker(&db->lock); // lock db in order to retrieve valid row id for insert
    execute();
    if (hasError()) {
        return -1;
    }
    return getLastRowId();
}

U2DataId SQLiteQuery::insert(U2DataType type, const QByteArray& dbExtra) {
    qint64 lastRowId = insert();
    if (hasError()) {
        return emptyId;
    }
    return SQLiteUtils::toU2DataId(lastRowId, type, dbExtra);
}



qint64 SQLiteQuery::selectInt64() {
    if (step()) {
        return getInt64(0);
    }
    if (!hasError()) {
        setError(SQLiteL10n::tr("Query produced no results: %1").arg(sql));
    }
    return -1;
}

qint64 SQLiteQuery::selectInt64(qint64 defaultValue) {
    if (step()) {
        return getInt64(0);
    }
    return defaultValue;
}


U2DataId SQLiteQuery::selectDataId(U2DataType type, const QByteArray& dbExtra) {
    if (step()) {
        return SQLiteUtils::toU2DataId(getInt64(1), type, dbExtra);
    }
    return emptyId;
}

QList<U2DataId> SQLiteQuery::selectDataIds(U2DataType type, const QByteArray& dbExtra) {
    QList<U2DataId> res;
    while(step()) {
        U2DataId id = getDataId(0, type, dbExtra);
        res.append(id);
    }
    return res;
}

QList<U2DataId> SQLiteQuery::selectDataIdsExt() {
    QList<U2DataId> res;
    while(step()) {
        U2DataType type = getDataType(1);
        QByteArray dbExtra = getCString(2);
        U2DataId id = getDataId(0, type, dbExtra);
        res.append(id);
    }
    return res;
}

QStringList SQLiteQuery::selectStrings() {
    QStringList res;
    while (step()) {
        QString text = getString(0);
        res.append(text);
    }
    return res;
}


qint64 SQLiteQuery::getLastRowId() {
    qint64 sqliteId = sqlite3_last_insert_rowid(db->handle);
    return sqliteId;
}

//////////////////////////////////////////////////////////////////////////
// SQLite transaction helper
SQLiteTransaction::SQLiteTransaction(DbRef* ref, U2OpStatus& _os)
: db(ref), os(_os) 
{
    if (db->useTransaction && db->transactionDepth == 0) {
        db->lock.lock();
        int rc = sqlite3_exec(db->handle, "BEGIN TRANSACTION;", NULL, NULL, NULL);
        if (rc != SQLITE_OK) {
            db->lock.unlock();
            os.setError(SQLiteL10n::queryError(sqlite3_errmsg(db->handle)));
            return;
        }

    }
    db->transactionDepth++;
}

SQLiteTransaction::~SQLiteTransaction() {
    if (db->useTransaction && db->transactionDepth == 1) {
        int rc;
        if (os.hasError()) {
            rc = sqlite3_exec(db->handle, "ROLLBACK TRANSACTION;", NULL, NULL, NULL);
        } else {
            rc = sqlite3_exec(db->handle, "COMMIT TRANSACTION;", NULL, NULL, NULL);
        }
        db->lock.unlock();
        if (rc != SQLITE_OK) {
            os.setError(SQLiteL10n::queryError(sqlite3_errmsg(db->handle)));
        }
    }
    db->transactionDepth--;
}


} //namespace
