#ifndef STATE_H
#define STATE_H

#include <QQuickItem>

class State : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QString topic MEMBER m_topic READ getTopic)
    Q_PROPERTY(QString defaultVal MEMBER m_defaultVal READ getDefaultVal)
    Q_PROPERTY(QString state MEMBER m_state READ getState)

private:
    QString m_topic;        // name of state
    QString m_defaultVal;   // default value of state
    QString m_state;        // current state


public:
    explicit State(QQuickItem *parent = 0);
    void componentComplete() Q_DECL_OVERRIDE;

    Q_INVOKABLE QString getTopic() {return m_topic;}
    Q_INVOKABLE QString getDefaultVal() {return m_defaultVal;}

    Q_INVOKABLE void setState(QString s) { m_state = s; }
    Q_INVOKABLE QString getState() { return m_state;}

signals:
    void stateUpdated();

public slots:
    Q_INVOKABLE void updateState(QString topic, QString newState);
    Q_INVOKABLE void updateState(QString newState);
};

#endif // STATE_H
