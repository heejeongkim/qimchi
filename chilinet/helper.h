#ifndef HELPER_H
#define HELPER_H

char* toChar(QString qStr){ //Convert from QString to char*
    int size = strlen(qStr.toUtf8().data())+1;
    char* cStr = (char*) malloc(size);
    memcpy(cStr, qStr.toUtf8().data(), size);
    cStr[size] = 0;
    return cStr;
}

#endif // HELPER_H
