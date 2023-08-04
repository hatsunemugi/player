#ifndef SQL_H
#define SQL_H

#include <QSql>
#include <QSqlError>
#include <QSqlField>
#include <QSqlIndex>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QSqlRelationalTableModel>

class Sql : public QObject
{
    Q_OBJECT
public:
    explicit Sql(QString);
    //数据库开关
    bool open(QSqlDatabase &data, QString path, QString connect);
    bool close(QSqlDatabase &data);

    //表格操作
    bool create_table(QSqlDatabase &data, QList<QString> list);
    bool rename_table(QSqlDatabase &data, QString old_name, QString new_name);
    bool delete_table(QSqlDatabase &data, QString name);

    //列操作
    bool add_column(QSqlDatabase &data, QString table_name, QString new_column);

    //行操作
    bool insert_row(QSqlDatabase &data, QList<QVariant> list);
    bool update_row(QSqlDatabase &data, QList<QVariant> list);
    bool delete_row(QSqlDatabase &data, QList<QVariant> list);

    //行列操作
    bool rename_id(QSqlDatabase &data, QList<QVariant> list);

    //获取数据
    QList<QVariant> get_row(QSqlDatabase &data, QList<QVariant> list);

    //其他
    int get_max_id(QSqlDatabase &data, QString table_name);
    int get_min_id(QSqlDatabase &data, QString table_name);

    static QSqlQuery exec(QSqlDatabase&,QString);

    static bool print(QSqlQuery);

public:
    QSqlDatabase database;
signals:
};

namespace Instance {
    static Sql* sql = nullptr;
    static QString path;
    static QString name;
    static Sql* getSqlInstance(QString value)
    {
        bool opened = path == value + ".db" && name == value;
        if(sql == nullptr)
            sql = new Sql(value);
        else
        {
            if(!opened){
                sql->open(sql->database,path,value);
            }
        }
        if(!opened){
            path = value + ".db";
            name = value;
        }
        return sql;
    }
};

inline Sql::Sql(QString name)/*:database(QSqlDatabase::database(name))*/
{
    open(database,name+".db",name);
}

inline bool Sql::open(QSqlDatabase &data, QString path, QString connect)
{
    if(close(data))
        qDebug() << "去除旧连接"<< connect;

    data = QSqlDatabase::addDatabase("QSQLITE", connect);
    data.setDatabaseName(path);
    if (!data.open())
    {
        qDebug() << "连接" << connect << "数据库失败" << data.lastError();
        return false;
    }
    else
    {
        qDebug() << "成功连接" << connect << "数据库" ;
        return true;
    }
}

inline bool Sql::close(QSqlDatabase &data)
{
    if(data.isOpen())
    {
            QString connection_name = data.connectionName();
            data.close();
            data = QSqlDatabase();
            data.removeDatabase(connection_name);
            return true;
    }
    else return false;
}

inline bool Sql::create_table(QSqlDatabase &data, QList<QString> list)
{
    QSqlQuery query(data);
    if(query.exec("SELECT * FROM " + list[0]))
    {
        qDebug() << "创建表" + list[0] + "失败，已经存在这个表";
        return false;
    }
    if(list.count()<2)
    {
        qDebug() << "创建表" + list[0] + "失败，请至少包含主键列";
        return false;
    }
    QString content;
    for(int i=0; i<list.count(); i++)
    {
        if(i==0)
            content = content.append("CREATE TABLE " + list[i] + "(");
        else if(list[i] == "KEY")
            content = content.append("id INTEGER PRIMARY KEY AUTOINCREMENT, ");
        else if(i>0 && i<list.count()-1 && list[i] != "KEY")
            content = content.append(list[i] + ", ");
        else if(i == list.count()-1 && list[i] != "KEY")
            content = content.append(list[i] + ")");
        else
            qDebug() << "数组遍历错误，请检查拼写";
    }
    qDebug()<<content;
    if(!query.exec(content))
    {
        qDebug() << "设置第一次创建表" + list[0] + "失败，请检查列和类型是否拼写错误"<< query.lastError();
        return false;
    }
    else
    {
        qDebug() << "设置第一次创建表" + list[0] + "成功";
        return true;
    }

}

inline bool Sql::rename_table(QSqlDatabase &data, QString old_name, QString new_name)
{
    QSqlQuery query(data);
    if(!query.exec("SELECT * FROM " + old_name))
    {
        qDebug() << "重命名表" + old_name + "失败，请检查是否存在这个表";
        return false;
    }
    if(!query.exec("ALTER TABLE " + old_name + " RENAME TO " + new_name))
    {
        qDebug() << "重命名表" + old_name + "失败"<< query.lastError();
        return false;
    }
    else
    {
        qDebug() << "重命名表" + old_name + "成功， 新表名：" + new_name;
        return true;
    }
}

inline bool Sql::delete_table(QSqlDatabase &data, QString name)
{
    QSqlQuery query(data);
    if(!query.exec("SELECT * FROM " + name))
    {
        qDebug() << "删除表" + name + "失败，请检查是否存在这个表";
        return false;
    }
    if(!query.exec("DROP TABLE IF EXISTS " + name))
    {
        qDebug() << "删除表" + name + "失败"<< query.lastError();
        return false;
    }
    else
    {
        qDebug() << "删除表" + name + "成功";
        return true;
    }
}

inline bool Sql::add_column(QSqlDatabase &data, QString table_name, QString new_column)
{
    QSqlQuery query(data);
    if(!query.exec("SELECT * FROM " + table_name))
    {
        qDebug() << "表" + table_name + "中添加新列失败，请检查是否存在这个表";
        return false;
    }
    if(!query.exec("ALTER TABLE " + table_name + " ADD COLUMN " + new_column))
    {
        qDebug() << "表" + table_name + "中添加新列" + new_column + "失败"<< query.lastError();
            return false;
    }
    else
    {
        qDebug() << "表" + table_name + "中添加新列" + new_column + "成功";
                        return true;
    }
}

inline bool Sql::insert_row(QSqlDatabase &data, QList<QVariant> list)
{
    QSqlQuery query(data);
    if(!query.exec("SELECT * FROM " + list[0].toString()))
    {
        qDebug() << "表" + list[0].toString() + "中插入行失败，请检查是否存在这个表";
        return false;
    }
    if(list.count()<3)
    {
        qDebug() << "表" + list[0].toString() + "中插入行失败，请至少包含（表名，{表头[X]，数值})三组数值";
        return false;
    }
    QString content, columns, values;
    for(int i=1; i<list.count(); i++)
    {
        if(i%2!=0)
        {
            if(i==list.count()-2)
            {
                columns.append(list[i].toString());
                values.append(":" + list[i].toString());
            }
            else
            {
                columns.append(list[i].toString() + ", ");
                values.append(":" + list[i].toString() + ", ");
            }
        }
    }
    content = "INSERT INTO " + list[0].toString() + " (" + columns + ") VALUES (" + values + ")";
    query.prepare(content);
    //    qDebug() << content;
    for(int i=1; i<list.count(); i++)
        if(i%2==0)
        {
            query.bindValue(":"+list[i-1].toString(), list[i]);
        }


    if(!query.exec())
    {
        qDebug() << "表" + list[0].toString() + "中插入行失败" << query.lastError();
        return false;
    }
    else
    {
        qDebug() << "表" + list[0].toString() + "中插入行成功";
        return true;
    }

}

inline bool Sql::update_row(QSqlDatabase &data, QList<QVariant> list)
{
    QSqlQuery query(data);
    if(!query.exec("SELECT * FROM " + list[0].toString()))
    {
        qDebug() << "表" + list[0].toString() + "中更新行失败，请检查是否存在这个表";
        return false;
    }
    if(list.count()<5)
    {
        qDebug() << "表" + list[0].toString() + "中更新行失败，请至少包含（表名，{表头[X]，数值}，{表头[Y]，数值}）五组数值";
        return false;
    }
    QString content, condition, values;
    for(int i=1; i<list.count(); i++)
    {
        if(i%2!=0)
        {
            if(i==list.count()-4)
                values.append(list[i].toString() + " = :" + list[i].toString());
            else if(i==list.count()-2)
                condition.append(list[i].toString() + " = :" + list[i].toString());
            else
                values.append(list[i].toString() + " = :" + list[i].toString() + ", ");
        }
    }
    content = "UPDATE " + list[0].toString() + " SET " + values + " WHERE " + condition;
    qDebug() << content;
    query.prepare(content);
    for(int i=1; i<list.count(); i++)
    {
        if(i%2==0)
        {
            query.bindValue(":"+list[i-1].toString(), list[i]);
        }
    }

    if(!query.exec())
    {
        qDebug() << "表" + list[0].toString() + "中更新行失败" << query.lastError();
            return false;
    }
    else
    {
        qDebug() << "表" + list[0].toString() + "中更新行成功";
        return true;
    }
}

inline bool Sql::delete_row(QSqlDatabase &data, QList<QVariant> list)
{
    QString table_name = list[0].toString();
    QString id_column = list[1].toString();
    int id = list[2].toInt();
    QSqlQuery query(data);
    if(!query.exec("SELECT * FROM " + table_name))
    {
        qDebug() << "表" + table_name + "中删除行失败，请检查是否存在这个表";
        return false;
    }
    QString id_str = QString::number(id);
    if(!query.exec("DELETE FROM " +table_name + " WHERE "+ id_column +" = " + id_str))
    {
        qDebug()<< "删除行失败" << query.lastError();
                                         return false;
    }
    else
    {
        qDebug()<<"已经删除ID为" << id_str << "的行";
            return true;
    }
}

inline bool Sql::rename_id(QSqlDatabase &data, QList<QVariant> list)
{
    QString table_name = list[0].toString();
    QString id_column = list[1].toString();
    int id = list[2].toInt();
    QSqlQuery query(data);
    if(!query.exec("SELECT * FROM " + table_name))
    {
        qDebug() << "表" + table_name + "中删除行失败，请检查是否存在这个表";
        return false;
    }
    QString id_str = QString::number(id);
    if(!query.exec("DELETE FROM " +table_name + " WHERE "+ id_column +" = " + id_str))
    {
        qDebug()<< "删除行失败" << query.lastError();
        return false;
    }
    else
    {
        qDebug()<<"已经删除ID为" << id_str << "的行";
        return true;
    }
}

inline QList<QVariant> Sql::get_row(QSqlDatabase &data, QList<QVariant> list)
{
    QSqlQuery query(data);
    if(!query.exec("SELECT * FROM " + list[0].toString()))
    {
        qDebug() << "表" + list[0].toString() + "中行获取数据失败，请检查是否存在这个表";
    }
    if(list.count()<4)
    {
        qDebug() << "表" + list[0].toString() + "中行获取数据失败，请至少包含（表名，{表头[X]}, {ID表头，ID[Y]})四组数值";
    }

    QString column;
    QList<QVariant> list_return;
    QString id_column = list[list.count()-2].toString();
    QString id = QString::number(list[list.count()-1].toInt());

    for(int i=1; i<list.count()-2; i++)
    {
        if(i==list.count()-3)
            column.append(list[i].toString());
        else
            column.append(list[i].toString() + ", ");
    }
    QString content;
    content = "SELECT  " + column + "  FROM " + list[0].toString() + " WHERE "+ id_column +" = " + id;
    qDebug() << content;
    if(!query.exec(content))
    {
        qDebug() << "获取指定获取行失败" << query.lastError();
    }
    else
    {
        while(query.next())
        {
            for(int i=0; i<list.count()-3; i++)
                list_return.append(query.value(i));
        }
        qDebug() << "获取指定获取行成功";
    }

    return list_return;
}

inline int Sql::get_max_id(QSqlDatabase &data, QString table_name)
{
    QSqlQuery query(data);
    if(!query.exec("SELECT * FROM " + table_name))
    {
        qDebug() << "表" + table_name + "中获取最大ID失败，请检查是否存在这个表";
    }

    int max_id = 0;
    query.prepare("SELECT max(id) FROM " + table_name);
    if (!query.exec())
    {
        qDebug() << "获取最大id失败" << query.lastError();
    }
    else
    {
        while (query.next())
        {
            max_id = query.value(0).toInt();
        }
        qDebug() << "获取最大id成功 " << max_id;
    }
    return max_id;
}

inline int Sql::get_min_id(QSqlDatabase &data, QString table_name)
{
    QSqlQuery query(data);
    if(!query.exec("SELECT * FROM " + table_name))
    {
        qDebug() << "表" + table_name + "中获取最小ID失败，请检查是否存在这个表";
    }

    int min_id = 0;
    query.prepare("SELECT min(id) FROM " + table_name);
    if (!query.exec())
    {
        qDebug() << "获取最小id失败" << query.lastError();
    }
    else
    {
        while (query.next())
        {
            min_id = query.value(0).toInt();
        }
        qDebug() << "获取最小id成功 " << min_id;
    }
    return min_id;
}

inline QSqlQuery Sql::exec(QSqlDatabase &data, QString sql)
{
    QSqlQuery query(data);
    qDebug()<<"exec"<<sql;
    if(!query.exec(sql))
    {
        qDebug() <<query.lastError();
    }
    return query;
}

inline bool Sql::print(QSqlQuery query)
{
    while(query.next())
    {
        for (int i= 0; i< query.record().count(); i++) {
            qDebug()<<query.value(i);
        }
    }
    return true;
}


#endif // SQL_H
