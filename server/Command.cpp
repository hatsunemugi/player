
#include "Command.h"
QRegularExpression Command::space = QRegularExpression("\\s+");
QVector<QString> Command::operators  = QVector<QString>{ "=",":=","+","-","*","/","%","!","!=","<","<=",">",">=" };
QVector<QString> Command::separators = QVector<QString>{ ",",";",".","\'","\"","(",")","{","}" };
QVector<QString> Command::keywords   = QVector<QString>{ "unset","set","write","ip","port","load","terminator","assert"};
QVector<Word> Command::result;
Command::State Command::state;
Command::Command(QObject *parent)
    : QObject{parent}
{
    qDebug()<<"entry";
}

QVector<Word> Command::analyse(QString input)
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

void Command::append(State cookie, QString body)
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

bool Command::isOperator(QString s)
{
    for(int i=0;i<operators.size();i++)
        if(operators[i]==s)
            return true;
    return false;
}

bool Command::isSeparator(QString s)
{
    for(int i=0;i<separators.size();i++)
        if(separators[i]==s)
            return true;
    return false;
}

bool Command::isKeyword(QString s)
{
    for(int i=0;i<keywords.size();i++)
        if(!keywords[i].compare(s,Qt::CaseInsensitive))
            return true;
    return false;
}

bool Command::isEmpty(QString input)
{
    QRegularExpressionMatch match=Command::space.match(input);
    if(match.hasMatch())
        return true;
    return false;
}


