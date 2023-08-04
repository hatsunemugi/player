
#ifndef COMMAND_H
#define COMMAND_H

#include <QRegularExpression>
#include <QObject>
#include <QVector>
#include <QQueue>
#include <QDebug>
struct Word{
    QString type;
    QString body;
    Word* prev;
    Word* next;
    friend QDebug operator <<(QDebug dbg, const Word& p)
    {
        dbg<<p.type<<p.body;
        return dbg;
    }
    friend bool operator ==(const Word& p, const QString& s)
    {
        return p.body==s;
    }
    friend bool operator ==(const QVector<Word>& cmd, const QString& s)
    {
        QStringList sl=s.split(" ");
        if(sl.back()=="")
            sl.pop_back();
//        qDebug()<<cmd.size()<<s<<sl.size();
        if(cmd.size()==sl.size())
        {
            //        qDebug()<<"length match";
            if(sl[0]=="set")
                return cmd[0]==sl[0]&&cmd[1]==sl[1];
            else if(sl[0]=="unset")
                return cmd[0]==sl[0]&&cmd[1]==sl[1];
            else if(sl[0]=="select")
                return cmd[0]==sl[0]&&cmd[2]==sl[2];
            else if(sl[0]=="play")
                return cmd[0]==sl[0]&&cmd[2]==sl[2];
            else
                return cmd[0]==sl[0];
        }
        return false;
    }
    friend QDebug operator <<(QDebug dbg, const QVector<Word>& a)
    {
        foreach (const Word& word, a) {
            qDebug()<<word;
        }
        return dbg;
    }
    friend void print(const QVector<Word>& a)
    {
        foreach (const Word& word, a) {
            qDebug()<<word;
        }
    }
};

class Command : public QObject
{
    Q_OBJECT
public:
    enum State{
        START = 1,
        IDENTIFIER_OR_KEYWORD = 2,
        IDENTIFIER=3,
        OPERATOR=4,
        SEPARATOR=5,
        NUMBER=6,
        DONE=7
    };
public:
    explicit Command(QObject *parent = nullptr);
    static QVector<Word> analyse(QString);
    static void append(State,QString);
    static bool isLetter(QString s){return ("a"<=s&&s<="z")||("A"<=s&&s<="Z")||(QChar(0x4e00)<=s[0]&&s[0]<=QChar(0x9fa5));}
    static bool isDigit(QString s){return "0"<=s&&s<="9";}
    static bool isOperator(QString);
    static bool isSeparator(QString);
    static bool isKeyword(QString);
    static bool isEmpty(QString);
private:
    inline static QRegularExpression space = QRegularExpression("^\\s+$");
    inline static QVector<QString> operators  = QVector<QString>{ "=",":=","+","-","*","/","%","!","!=","<","<=",">",">=" };
    inline static QVector<QString> separators = QVector<QString>{ ",",";",".","\'","\"","(",")","{","}" };
    inline static QVector<QString> keywords   = QVector<QString>{ "unset","set","write","ip","port","load","terminator","assert"};
    inline static QVector<Word> result ;
    inline static Command::State state;
};

inline Command::Command(QObject *parent)
    : QObject{parent}
{
    qDebug()<<"entry";
}

inline QVector<Word> Command::analyse(QString input)
{
    assert(!Command::isEmpty(input));
    result.clear();
    QVector<Word> e;
    result.swap(e);

    input+="$";
    QQueue<QString> queue;
    for(int i=0;i<input.length();i++)
        queue.enqueue(input.mid(i,1));
    QString s="",t="";
    bool save;
    int Dot=0;
    state=START;
    State cookie=state;
    while(!queue.isEmpty()&&state!=DONE)
    {
        t=queue.front();
        save=true;
        switch(state){
        case START:
            if(isDigit(t))
                state=NUMBER;
            else if(isLetter(t))
                state=IDENTIFIER_OR_KEYWORD;
            else if(t==" "||t=="\t"||t=="\n")
                save=false,queue.dequeue();
            else if(isOperator(t))
                state=OPERATOR;
            else if(isSeparator(t))
                state=SEPARATOR;
            break;
        case NUMBER:
            if(isDigit(t))
                break;
            else if(t=="."&&Dot==0)
                Dot=1;
            else
                cookie=state,state=DONE,save=false;
            break;
        case IDENTIFIER_OR_KEYWORD:
            if(isDigit(t))
                state=IDENTIFIER;
            else if(isLetter(t)||t=="_")
                state=IDENTIFIER_OR_KEYWORD;
            else
                cookie=state,state=DONE,save=false;
            break;
        case IDENTIFIER:
            if(isDigit(t)||isLetter(t)||t=="_")
                state=IDENTIFIER;
            else
                cookie=state,state=DONE,save=false;
            break;
        case SEPARATOR:
            if(isSeparator(s+t))
                state=DONE;
            else
                cookie=state,state=DONE,save=false;
            break;
        case OPERATOR:
            if(isOperator(s+t))
                state=OPERATOR;
            else
                cookie=state,state=DONE,save=false;
            break;
        case DONE:
            break;
        }
        if(save==true)
            s+=t,queue.dequeue();
        if(state==DONE)
            append(cookie,s),s.clear(),Dot=0,state=START;
    }
    result[0].prev=nullptr;
    result.back().next=nullptr;
    for(int i=0;i<result.size()-1;i++)
    {
        result[i].next=&result[i+1];
        result[i+1].prev=&result[i];
    }

    Word* word=&result[0];
    Word* quote = nullptr;
    bool buffered=false;
    QString buffer;
    int n=0;
    while(word!=nullptr&&n<result.size())
    {
        word=&result[n];
        QString type=word->type;
        QString body=word->body;
//        qDebug()<<*word;
        if(!buffered)
        {
            if(type=="分隔符")
            {
                if(body=="\'"||body=="\"")
                {
//                    qDebug()<<"<<'";
                    buffered=true;
                    quote = word;
                }
            }
            n++;
        }
        else
        {
            buffer+=body;
            result.removeAt(n);
//            qDebug()<<body<<n<<"removed";
            if(type=="分隔符")
            {
                if(quote!=nullptr&&quote->body==body)
                {
//                    qDebug()<<">>'";
                    buffered=false;
                    buffer.chop(1);
                    quote->body=buffer;
                    quote->type="字符串";
                    if(word->next==nullptr)
                    {
                        quote->next=nullptr;
                        return result;
                    }
                    else
                    {
                        quote->next=word->next;
                        word->next->prev=quote;
                        quote=nullptr;
                        buffer.clear();
                    }
                }
            }
//            else
//                qDebug()<<"<<"<<body;
            if(n==result.size())
            {
                if(quote!=nullptr)
                {
//                    qDebug()<<"incomplete";
                    quote->body=buffer;
                    quote->type="字符串";
                    quote->next=nullptr;
                    return result;
                }
            }
        }
    }

    return result;
}

inline void Command::append(State cookie, QString body)
{
    QString type;
    if(cookie==NUMBER)
        type="数值";
    if(cookie==IDENTIFIER)
        type="标识符";
    if(cookie==OPERATOR)
        type="操作符";
    if(cookie==SEPARATOR){
        if(body=="/*"||body=="//"||body=="*/") type="注释";
        else type="分隔符";}
    if(cookie==IDENTIFIER_OR_KEYWORD){
        if(isKeyword(body)) type="关键词";
        else type="标识符";}
    result.push_back(Word{type,body,nullptr,nullptr});
}

inline bool Command::isOperator(QString s)
{
    for(int i=0;i<operators.size();i++)
        if(operators[i]==s)
            return true;
    return false;
}

inline bool Command::isSeparator(QString s)
{
    for(int i=0;i<separators.size();i++)
        if(separators[i]==s)
            return true;
    return false;
}

inline bool Command::isKeyword(QString s)
{
    for(int i=0;i<keywords.size();i++)
        if(!keywords[i].compare(s,Qt::CaseInsensitive))
            return true;
    return false;
}

inline bool Command::isEmpty(QString input)
{
    QRegularExpressionMatch match=space.match(input);
    if(match.hasMatch())
        return true;
    return false;
}
#endif // COMMAND_H
