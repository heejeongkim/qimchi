#include "state.h"
#include "chiliclient.h"


State::State(QQuickItem *parent) : QQuickItem(parent){

}

void State::componentComplete(){

}

void State::updateState(QString topic, QString newState){
    if(topic.compare(m_topic) == 0){
        m_state = newState;
        emit stateUpdated();
    }
 }

void State::updateState(QString newState){
    m_state = newState;
    emit stateUpdated();
 }
