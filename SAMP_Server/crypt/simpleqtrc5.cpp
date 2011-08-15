/*
 *  SimpleQtRC5 is an RC5 encryption library for Qt.
 *
 *  Copyright (C) 2010 Gunnar Thorburn
 *
 *  SimpleQtRC5 is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  ParrotShare is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "simpleqtrc5.h"

#include <QString>
#include <QCryptographicHash>
#include <QtEndian>
#include <QDate>
#include <QTime>

#include <QDebug>


#define ROUNDS 32
#define KEYSIZE 20
#define SSIZE 66

#define P32 0xb7e17163
#define Q32 0x9e3779b9
#define P64 Q_UINT64_C(0xb7e151628aed2a6b);
#define Q64 Q_UINT64_C(0x9e3779b97f4a7c15);

#define ROTL32(x,y) (((x)<<(y)) | ((x)>>(32-(y))))
#define ROTR32(x,y) (((x)>>(y)) | ((x)<<(32-(y))))
#define ROTL64(x,y) (((x)<<(y)) | ((x)>>(64-(y))))
#define ROTR64(x,y) (((x)>>(y)) | ((x)<<(64-(y))))

namespace SimpleQtRC5 {


QByteArray header_RC5_32_32_20  = QString("RC5/32/32/20:").toAscii();
QByteArray header_RC5_64_32_20  = QString("RC5/64/32/20:").toAscii();

QByteArray header_CBC  = QString("CBC:PADN::").toAscii();
QByteArray header_CFB  = QString("CFB::").toAscii();


Algorithm Info::fastRC5() {
    #if (QT_POINTER_SIZE==4)
        return RC5_32_32_20;
    #else
        return RC5_64_32_20;
    #endif
}

QString Info::errorText(Error e) {
    switch (e) {
    case NoError:
        return QString("NoError");
    case ErrorNoAlgorithm:
        return QString("ErrorNoAlgorithm");
    case ErrorNoMode:
        return QString("ErrorNoMode");
    case ErrorInvalidKey:
        return QString("ErrorInvalidKey");
    case ErrorNotEnoughData:
        return QString("ErrorNotEnoughData");
    case ErrorModeNotImplemented:
        return QString("ErrorModeNotImplemented");
    case ErrorAlgorithmNotImplemented:
        return QString("ErrorAlgorithmNotImplemented");
    case ErrorChecksumNotImplemented:
        return QString("ErrorChecksumNotImplemented");
    case ErrorAlreadyError:
        return QString("ErrorAlreadyError");
    default:
        return QString("UnknownError");
    }
    return QString();
}


Key::Key() {
    // key = QByteArray()
    s32 = 0;
    s64 = 0;
    // qDebug() << "KEY++";
}

Key::Key(const QByteArray &k) {
    key = k;
    s32 = 0;
    s64 = 0;
    // qDebug() << "KEY++";
}

Key::Key(const QString &k) {
    QCryptographicHash qch(QCryptographicHash::Sha1);
    qch.addData(k.toUtf8());
    key = qch.result();
    s32 = 0;
    s64 = 0;
    // qDebug() << "KEY++";
}

Key::~Key() {
    delete[] s32;
    delete[] s64;
    // qDebug() << "KEY--";
}


void Key::expandKeyXX() {
    if (key.size() != KEYSIZE) {
        QByteArray newKey(KEYSIZE, 0);
        unsigned char *ok = (unsigned char *)(key.data());
        unsigned char *nk = (unsigned char *)(newKey.data());
        for ( int i = 0 ; i < key.size() ; i++ ) {
            nk[i%KEYSIZE] ^= ok[i];
        }
        key = newKey;
    }
}

void Key::expandKey32() {
    if (s32) return;
    expandKeyXX();
    s32 = new quint32[SSIZE];
    quint32 *s = s32;

    unsigned char *k = (unsigned char *)(key.data());
    quint32 L[5];
    L[0] = qFromLittleEndian<quint32>(k);
    L[1] = qFromLittleEndian<quint32>(k+4);
    L[2] = qFromLittleEndian<quint32>(k+8);
    L[3] = qFromLittleEndian<quint32>(k+12);
    L[4] = qFromLittleEndian<quint32>(k+16);

    s[0] = P32;
    for ( int i = 1 ; i < SSIZE ; i++ ) {
        s[i] = s[i-1] + Q32;
    }

    int i=0 , j=0;
    quint32 A=0 , B=0;

    for ( int x = 0 ; x < ROUNDS ; x++ ) {
        A = s[i] = ROTL32(s[i] + (A+B), 3);
        B = L[j] = ROTL32(L[j] + (A+B), (A+B) & 31 );
        i = (i + 1) % SSIZE;
        j = (j + 1) % 5;
    }
}

void Key::expandKey64() {
    if (s64) return;
    expandKeyXX();
    s64 = new quint64[SSIZE];
    quint64 *s = s64;

    unsigned char *k = (unsigned char *)(key.data());
    quint64 L[3];
    L[0] = qFromLittleEndian<quint64>(k);
    L[1] = qFromLittleEndian<quint64>(k+8);
    L[2] = qFromLittleEndian<quint32>(k+16);

    s[0] = P64;
    for ( int i = 1 ; i < SSIZE ; i++ ) {
        s[i] = s[i-1] + Q64;
    }

    int i=0 , j=0;
    quint64 A=0 , B=0;

    for ( int x = 0 ; x < ROUNDS ; x++ ) {
        A = s[i] = ROTL64(s[i] + (A+B), 3);
        B = L[j] = ROTL64(L[j] + (A+B), (A+B) & 63 );
        i = (i + 1) % SSIZE;
        j = (j + 1) % 3;
    }
}



/* *** ENCRYPTOR *** */

Encryptor::Encryptor(Key* k, Algorithm a, Mode m, Checksum c) {
    key = k;
    algorithm = a;
    mode = m;
    checksum = c;
    if ( algorithm == RC5_FAST_32_20 ) {
        algorithm = Info::fastRC5();
    }
    state = StateReset;
    modex = 0;
}

Encryptor::~Encryptor() {
    delete modex;
}

Error Encryptor::encrypt(const QByteArray &plain, QByteArray &cipher, bool end) {
    QByteArray tmpIn;
    switch ( state ) {
    case StateReset:

        switch ( algorithm ) {
        case RC5_32_32_20:
            tmpIn.append(header_RC5_32_32_20);
            break;
        case RC5_64_32_20:
            tmpIn.append(header_RC5_64_32_20);
            break;
        case NoAlgorithm:
        case DetectAlgorithm:
        case RC5_FAST_32_20:
            state = StateError;
            return ErrorNoAlgorithm;
        }

        switch ( mode ) {
        case ModeCBC:
            tmpIn.append(header_CBC);
            if ( 0 == modex) modex = new CBC(key,algorithm);
            break;
        case ModeCFB:
            tmpIn.append(header_CFB);
            if ( 0 == modex) modex = new CFB(key, algorithm);
            break;
        case NoMode:
        case DetectMode:
        default:
            state = StateError;
            return ErrorNoMode;
        }

        if ( checksum != NoChecksum ) {
            state = StateError;
            return ErrorChecksumNotImplemented;
        }
        // switch (checksum) HERE

        state = StateOn;
    case StateOn:
        tmpIn.append(plain);
        cipher = modex->encrypt(tmpIn, end);
        break;
    case StateError:
    default:
        return ErrorAlreadyError;    }

    if (end) {
        state = StateReset;
    }

    return NoError;
}


/* *** DECRYPTOR *** */

Decryptor::Decryptor(Key* k, Algorithm a, Mode m) {
    key = k;
    algorithm = a;
    mode = m;
    state = StateReset;
    checksum = NoChecksum;
    modex = 0;
}

Decryptor::~Decryptor() {
    delete modex;
}

Checksum Decryptor::getChecksumType() {
    return checksum;
}

Error Decryptor::decrypt(const QByteArray &cipher, QByteArray &plain, bool end) {
    QByteArray expectHeader;
    QByteArray remainingFromHeader;
    QByteArray tmpIn;
    QByteArray tmpOut;
    int neededForHeader = -1;
    int neededForIv = -1;

    switch ( state ) {
    case StateReset:
        switch ( algorithm ) {
        case RC5_32_32_20:
            expectHeader.append(header_RC5_32_32_20);
            neededForIv = 8;
            break;
        case RC5_64_32_20:
            expectHeader.append(header_RC5_64_32_20);
            neededForIv = 16;
            break;
        case NoAlgorithm:
        case DetectAlgorithm:
        case RC5_FAST_32_20:
            state = StateError;
            return ErrorNoAlgorithm;
        }

        switch ( mode ) {
        case ModeCBC:
            expectHeader.append(header_CBC);
            neededForHeader = (((neededForIv + expectHeader.size() - 1) / neededForIv) + 1) * neededForIv;
            if ( 0 == modex) modex = new CBC(key, algorithm);
            break;
        case ModeCFB:
            expectHeader.append(header_CFB);
            neededForHeader = neededForIv + expectHeader.size();
            if ( 0 == modex) modex = new CFB(key, algorithm);
            break;
        case NoMode:
        case DetectMode:
        default:
            state = StateError;
            return ErrorNoMode;
        }


        if ( cipher.size() < neededForHeader ) {
            state = StateError;
            return ErrorNotEnoughData;
        }

        tmpOut = modex->decrypt(cipher.left(neededForHeader), false);

        if ( tmpOut.startsWith(expectHeader) ) {
            remainingFromHeader = tmpOut.right(tmpOut.size() - expectHeader.size());
            tmpOut.clear();
            tmpIn = cipher.right(cipher.size() - neededForHeader);
            state = StateOn;
        } else {
            state = StateError;
            tmpOut.clear();
            return ErrorInvalidKey;
        }

    case StateOn:
        if ( tmpIn.isEmpty() ) {
            tmpIn = cipher;
        }
        tmpOut = modex->decrypt(tmpIn, end);
        break;
    case StateError:
    default:
        return ErrorAlreadyError;
    }
    if ( ! remainingFromHeader.isEmpty() ) {
        tmpOut.prepend(remainingFromHeader);
    }
    if (end) {
        state = StateReset;
    }
    plain = tmpOut;
    return NoError;
}


/* *** DECRYPTOR WIZARD ENTRY *** */

class DecryptorWizardEntry {
public:
    Key* key;
    Algorithm alg;
    Mode mode;
    Checksum csum;
};


/* *** DECRYPTOR WIZARD *** */

DecryptorWizard::DecryptorWizard() {
}

DecryptorWizard::DecryptorWizard(Key* k, Algorithm a, Mode m) {
    addParameters(k, a, m);
}

DecryptorWizard::~DecryptorWizard() {
    for ( int i=0 ; i < entries.size() ; i++ ) {
        delete entries.at(i);
    }
}

void DecryptorWizard::addParameters(Key* k, Algorithm a, Mode m) {
    DecryptorWizardEntry *dwe = new DecryptorWizardEntry();
    dwe->key = k;
    dwe->alg = a;
    dwe->mode = m;
    entries.append(dwe);
}

Error DecryptorWizard::decrypt(const QByteArray &cipher, QByteArray &plain, Decryptor* decryptor, bool end) {
    Algorithm aList[2] = { RC5_32_32_20, RC5_64_32_20 };
    Mode mList[2] = { ModeCBC, ModeCFB };
    int aL = 2;
    int mL = 2;
    int eL = entries.size();
    int eI, aI, mI;
    Decryptor *dx;
    Error dxError;
    Error retError = ErrorInvalidKey;

    for (eI=0 ; eI<eL ; eI++) for (aI=0 ; aI<aL ; aI++) for (mI=0 ; mI<mL ; mI++) {
        if ( (entries.at(eI)->alg != aList[aI]) && (entries.at(eI)->alg != DetectAlgorithm) ) continue;
        if ( (entries.at(eI)->mode != mList[mI]) && (entries.at(eI)->mode != DetectMode) ) continue;
        dx = new Decryptor(entries.at(eI)->key, aList[aI], mList[mI]);
        dxError = dx->decrypt(cipher, plain, end);
        switch (dxError) {
        case NoError:
            decryptor = dx;
            return NoError;
        case ErrorNotEnoughData:
            retError = ErrorNotEnoughData;
            break;
        case ErrorInvalidKey:
            if ( ErrorNotEnoughData != retError ) {
                retError = ErrorInvalidKey;
            }
            break;
        default:
            delete dx;
            return dxError;
        }
        delete dx;
    }
    return retError;
}

Error DecryptorWizard::decryptToEnd(const QByteArray &cipher, QByteArray &plain) {
    Decryptor* qspd=new Decryptor();
    Error er = decrypt(cipher, plain, qspd, true);
    return er;
}


/* *** INITIALIZATION VECTOR *** */

InitializationVector *InitializationVector::singleInstance = 0;

void InitializationVector::fixReserved(QByteArray &v) {
    int s;
    int vs = v.size();
    int rs = reservedVectors.size();
    QByteArray cv;
    while ( true ) {
        start_over:
        for ( int i = 0 ; i < rs ; i++ ) {
            cv = reservedVectors.at(i);
            s = qMin(vs, cv.size());
            if ( v.left(s) == cv.left(vs) ) {
                v[qrand() % s] = (uchar)(qrand() % 256);
                goto start_over;
            }
        }
        return;
    }
}

QByteArray InitializationVector::getVector8() {
    initiate();
    QByteArray ret(8, 0);
    quint32 A = ((quint32)(qrand())) ^ ((quint32)(QTime::currentTime().msecsTo(QTime(23,59,59,999))));
    quint32 B = ((quint32)(qrand())) ^ ((quint32)(QDate::currentDate().daysTo(QDate(2999,12,31))));
    qToLittleEndian(A, (uchar *)(ret.data()));
    qToLittleEndian(B, (uchar *)(ret.data() + 4));
    singleInstance->fixReserved(ret);
    return ret;
}

QByteArray InitializationVector::getVector16() {
    initiate();
    QByteArray ret(16, 0);
    quint32 A = ((quint32)(qrand())) ^ ((quint32)(QTime::currentTime().msecsTo(QTime(23,59,59,999))));
    quint32 B = ((quint32)(qrand())) ^ ((quint32)(QDate::currentDate().daysTo(QDate(2999,12,31))));
    quint32 C = (quint32)(qrand());
    quint32 D = (quint32)(qrand());
    qToLittleEndian(A, (uchar *)(ret.data()));
    qToLittleEndian(B, (uchar *)(ret.data() + 4));
    qToLittleEndian(C, (uchar *)(ret.data() + 8));
    qToLittleEndian(D, (uchar *)(ret.data() + 12));
    singleInstance->fixReserved(ret);
    return ret;
}

void InitializationVector::reserveVector(QByteArray rv) {
    initiate();
    singleInstance->reservedVectors.append(rv);
}

void InitializationVector::initiate() {
    if ( 0 == singleInstance ) {
        singleInstance = new InitializationVector();
    }
}

InitializationVector::InitializationVector() {
    qsrand((quint32)(QTime::currentTime().msecsTo(QTime(23,59,59,999))));
}

InitializationVector::~InitializationVector() {
}


/* *** CBC *** */

CBC::CBC(Key* k, Algorithm a) {
    algorithm = a;
    key = k;
    reset();
}

CBC::~CBC() {
}

void CBC::reset() {
    worksize = -1;
    buffer.clear();
    cbcBuffer.clear();
    padHostageBuffer.clear();
}

/*
 *
 * PSEUDO
 *
 *
 *
 *
 *
 *
 *
 */
QByteArray CBC::encrypt(const QByteArray plain, bool end) {
    int cipherpos = 0;
    int padsize = -1;
    bool iv = false;

    // set initialization vector if first data
    if ( -1 == worksize ) {
        switch (algorithm) {
        case RC5_32_32_20:
            cbcBuffer = InitializationVector::getVector8();
            worksize = 8;
            iv = true;
            key->expandKey32();
            break;
        case RC5_64_32_20:
            cbcBuffer = InitializationVector::getVector16();
            worksize = 16;
            iv = true;
            key->expandKey64();
            break;
        default:
            buffer.clear();
            return QByteArray();
        }
    }
    buffer.append(plain);

    int cipherlen = ( buffer.size() / worksize ) * worksize;
    padsize = worksize - (buffer.size() - cipherlen);
    if (iv) cipherlen += cbcBuffer.size();
    if (end) cipherlen += worksize;

    QByteArray cipher = QByteArray(cipherlen, 0);

    if (end) {
        buffer.append(QByteArray(padsize, (char)padsize));
    }

    if ( iv ) while (cipherpos < worksize) {
        cipher[cipherpos] = cbcBuffer[cipherpos];
        cipherpos++;
    }

    uchar *bufdat = (uchar *)buffer.data();
    uchar *cipdat = (uchar *)cipher.data();
    uchar *cbcdat = (uchar *)cbcBuffer.data();
    int bufpos = 0;

    while ( cipherpos < cipherlen ) {
        for ( int i=0 ; i < worksize ; i++ ) {
            cbcdat[i] ^= bufdat[i + bufpos];
        }
        switch (algorithm) {
        case RC5_32_32_20:
            {
            quint32 X32_1 = qFromLittleEndian<quint32>(cbcdat);
            quint32 X32_2 = qFromLittleEndian<quint32>(cbcdat + 4);
            rc5_32_encrypt_2w(X32_1,X32_2,key->s32);
            qToLittleEndian(X32_1, cbcdat);
            qToLittleEndian(X32_2, cbcdat + 4);
            }
            break;
        case RC5_64_32_20:
            {
            quint64 X64_1 = qFromLittleEndian<quint64>(cbcdat);
            quint64 X64_2 = qFromLittleEndian<quint64>(cbcdat + 8);
            rc5_64_encrypt_2w(X64_1,X64_2,key->s64);
            qToLittleEndian(X64_1, cbcdat);
            qToLittleEndian(X64_2, cbcdat + 8);
            }
            break;
        default:
            cipher.clear();
            return cipher;
        }
        for ( int i=0 ; i < worksize ; i++ ) {
            cipdat[cipherpos + i] = cbcdat[i];
        }
        cipherpos += worksize;
        bufpos += worksize;
    }

    if (end) {
        reset();
    } else {
        buffer = buffer.right( buffer.size() - bufpos );
    }
    return cipher;
}

QByteArray CBC::decrypt(const QByteArray cipher, bool end) {
    int bufferpos = 0;
    int plainpos = 0;

    buffer.append(cipher);
    if ( -1 == worksize ) {
        switch (algorithm) {
        case RC5_32_32_20:
            if ( buffer.size() < 8 ) return QByteArray();
            cbcBuffer = buffer.left(8);
            worksize = 8;
            bufferpos = 8;
            key->expandKey32();
            break;
        case RC5_64_32_20:
            if ( buffer.size() < 16 ) return QByteArray();
            cbcBuffer = buffer.left(16);
            worksize = 16;
            bufferpos = 16;
            key->expandKey64();
            break;
        default:
            buffer.clear();
            return QByteArray();
        }
    }

    int plainlen = ( (buffer.size() - bufferpos) / worksize ) * worksize + padHostageBuffer.size();
    QByteArray plain(plainlen, 0);
    while (plainpos < padHostageBuffer.size()) {
        plain[plainpos] = padHostageBuffer[plainpos];
        plainpos++;
    }
    padHostageBuffer.clear();

    uchar *bufdat = (uchar *)buffer.data();
    uchar *plndat = (uchar *)plain.data();
    uchar *cbcdat = (uchar *)cbcBuffer.data();


    while ( plainpos < plainlen ) {
        switch (algorithm) {
        case RC5_32_32_20:
            {
            quint32 X32_1 = qFromLittleEndian<quint32>(bufdat + bufferpos);
            quint32 X32_2 = qFromLittleEndian<quint32>(bufdat + bufferpos + 4);
            rc5_32_decrypt_2w(X32_1,X32_2,key->s32);
            qToLittleEndian(X32_1, plndat + plainpos);
            qToLittleEndian(X32_2, plndat + plainpos + 4);
            }
            break;
        case RC5_64_32_20:
            {
            quint64 X64_1 = qFromLittleEndian<quint64>(bufdat + bufferpos);
            quint64 X64_2 = qFromLittleEndian<quint64>(bufdat + bufferpos + 8);
            rc5_64_decrypt_2w(X64_1,X64_2,key->s64);
            qToLittleEndian(X64_1, plndat + plainpos);
            qToLittleEndian(X64_2, plndat + plainpos + 8);
            }
            break;
        default:
            plain.clear();
            return plain;
        }

        for ( int i=0 ; i < worksize ; i++ ) {
            plndat[plainpos + i] ^= cbcdat[i];
            cbcdat[i] = bufdat[bufferpos + i];
        }
        bufferpos += worksize;
        plainpos += worksize;
    }

    if (end) {
        // in case we dont have any valid padding, the only explanation
        // is a transmission error, or someone modified the file
        // however, this is not the layer where to discover such problems
        // and I have nowhere to report a problem, so I just need
        // to avoid crashing
        int padc = 0;
        if ( ! plain.isEmpty() ) {
            padc = (int)plain[plainlen - 1];
            if ( padc > plain.size() ) {
                padc = 0;
            }
        }
        if ( 0 < padc && padc <= 16 ) {
            plain = plain.left(plainlen - padc);
        }
        reset();
    } else {
        buffer = buffer.right(buffer.size() - bufferpos);

        // there is a chance, that we will not get more data,
        // but end=true anyways. in this case, we must not
        // return possible pad data as plain text
        if ( buffer.size() == 0 && plainlen > 0 ) {
            uchar lastByte = plain[plainlen - 1];
            if ( 0 < lastByte && lastByte <= 16 ) {
                QByteArray padPattern((int)lastByte,(char)lastByte);
                if (plain.endsWith(padPattern)) {
                    padHostageBuffer = plain.right(worksize);
                    plain = plain.left(plainlen - worksize);
                    plainlen -= worksize;
                }
            }
        }
    }

    return plain;
}


/* *** CFB *** */

CFB::CFB(Key* k, Algorithm a) {
    algorithm = a;
    key = k;
    reset();
}

CFB::~CFB() {
}

void CFB::reset() {
    bufferpos = -1;
    buffer.clear();
}


/*
 * PSEUDO:
 *
 * if first: make IV, put in buffer
 *
 * if unused bytes in buffer
 *   cipher = buffer XOR plain
 *   buffer = cipher
 *
 * while ( more to encrypt )
 *   encrypt buffer
 *   cipher = buffer XOR plain
 *   buffer = cipher
 *
 *
 */
QByteArray CFB::encrypt(const QByteArray plain, bool end) {
    int plainpos = 0;
    int plainlen = plain.size();
    int cipherpos = 0;
    int bufferlen = buffer.size();
    int copysize = 0;
    QByteArray cipher(plainlen, 0);
    uchar *bufdat = 0;

    // set initialization vector if first data
    if ( -1 == bufferpos ) {
        switch (algorithm) {
        case RC5_32_32_20:
            buffer = InitializationVector::getVector8();
            key->expandKey32();
            bufferlen = 8;
            break;
        case RC5_64_32_20:
            buffer = InitializationVector::getVector16();
            key->expandKey64();
            bufferlen = 16;
            break;
        default:
            buffer.clear();
            return QByteArray();
        }
        cipher.prepend(buffer);
        bufferpos = bufferlen;
        cipherpos += bufferlen;
    }

    bufdat = (uchar *)(buffer.data());
    copysize = qMin( bufferlen - bufferpos , plainlen - plainpos );
    // in case the buffer contains unused data from last encrypt,
    // use those bytes first
    while ( 0 < copysize ) {
        bufdat[bufferpos] = plain[plainpos] ^ bufdat[bufferpos];
        cipher[cipherpos] = bufdat[bufferpos];
        cipherpos++;
        plainpos++;
        bufferpos++;
        copysize--;
    }

    copysize = qMin( bufferlen , plainlen - plainpos );
    while ( 0 < copysize ) {
        switch (algorithm) {
        case RC5_32_32_20:
            {
            quint32 X32_1 = qFromLittleEndian<quint32>(bufdat);
            quint32 X32_2 = qFromLittleEndian<quint32>(bufdat + 4);
            rc5_32_encrypt_2w(X32_1,X32_2,key->s32);
            qToLittleEndian(X32_1, bufdat);
            qToLittleEndian(X32_2, bufdat + 4);
            }
            break;
        case RC5_64_32_20:
            {
            quint64 X64_1 = qFromLittleEndian<quint64>(bufdat);
            quint64 X64_2 = qFromLittleEndian<quint64>(bufdat + 8);
            rc5_64_encrypt_2w(X64_1,X64_2,key->s64);
            qToLittleEndian(X64_1, bufdat);
            qToLittleEndian(X64_2, bufdat + 8);
            }
            break;
        default:
            cipher.clear();
            return cipher;
        }
        bufferpos = 0;

        // optimization opportunity here by working on words instead of buffers
        while ( 0 < copysize ) {
            bufdat[bufferpos] = plain[plainpos] ^ bufdat[bufferpos];
            cipher[cipherpos] = bufdat[bufferpos];
            cipherpos++;
            plainpos++;
            bufferpos++;
            copysize--;
        }
        copysize = qMin( bufferlen , plainlen - plainpos );
    }
    if (end) {
        reset();
    }
    return cipher;
}



/*
 * PSEUDO
 *
 * if buffer not full-size
 *   copy cipher (IV) into buffer
 *
 * if unused buffer available
 *   plain = cipher XOR buffer
 *   buffer = cipher
 *
 * while ( more to decrypt )
 *   encrypt buffer
 *   plain = cipher XOR buffer
 *   buffer = cipher
 *
 */
QByteArray CFB::decrypt(const QByteArray cipher, bool end) {
    int cipherpos = 0;
    int cipherlen = cipher.size();
    int bufferlen = -1;
    int copysize = 0;
    int plainpos = 0;
    uchar *bufdat = 0;

    // as long as bufferpos == -1, the initialization vector
    // has not yet been loaded
    if ( -1 == bufferpos ) {
        switch (algorithm) {
        case RC5_32_32_20:
            key->expandKey32();
            bufferlen = 8;
            break;
        case RC5_64_32_20:
            key->expandKey64();
            bufferlen = 16;
            break;
        default:
            return QByteArray();
        }
        copysize = qMin ( bufferlen - buffer.size() , cipherlen );
        buffer.append(cipher.left(copysize));
        cipherpos = copysize;
        if ( bufferlen == buffer.size() ) {
            bufferpos = bufferlen;
        } else {
            return QByteArray();
        }
    } else {
        bufferlen = buffer.size();
    }

    QByteArray plain(cipherlen - cipherpos, 0);

    copysize = qMin( bufferlen - bufferpos , cipherlen - cipherpos );
    while ( 0 < copysize ) {
        plain[plainpos] = buffer[bufferpos] ^ cipher[cipherpos];
        buffer[bufferpos] = cipher[cipherpos];
        plainpos++;
        cipherpos++;
        bufferpos++;
        copysize--;
    }

    bufdat = (uchar *)(buffer.data());
    copysize = qMin( bufferlen , cipherlen - cipherpos );
    while ( 0 < copysize ) {
        switch (algorithm) {
        case RC5_32_32_20:
            {
            quint32 X32_1 = qFromLittleEndian<quint32>(bufdat);
            quint32 X32_2 = qFromLittleEndian<quint32>(bufdat + 4);
            rc5_32_encrypt_2w(X32_1,X32_2,key->s32);
            qToLittleEndian(X32_1, bufdat);
            qToLittleEndian(X32_2, bufdat + 4);
            }
            break;
        case RC5_64_32_20:
            {
            quint64 X64_1 = qFromLittleEndian<quint64>(bufdat);
            quint64 X64_2 = qFromLittleEndian<quint64>(bufdat + 8);
            rc5_64_encrypt_2w(X64_1,X64_2,key->s64);
            qToLittleEndian(X64_1, bufdat);
            qToLittleEndian(X64_2, bufdat + 8);
            }
            break;
        default:
            plain.clear();
            return plain;
        }
        bufferpos = 0;

        while ( 0 < copysize ) {
            plain[plainpos] = buffer[bufferpos] ^ cipher[cipherpos];
            buffer[bufferpos] = cipher[cipherpos];
            plainpos++;
            cipherpos++;
            bufferpos++;
            copysize--;
        }

        copysize = qMin( bufferlen , cipherlen - cipherpos );
    }
    if (end) {
        reset();
    }
    return plain;
}






void rc5_32_encrypt_2w(quint32 &X1, quint32 &X2, const quint32 *s) {
    quint32 x1 = X1 + s[0];
    quint32 x2 = X2 + s[1];
    int r;
    for ( int i = 1 ; i <= ROUNDS ; i++ ) {
        r = x2 & 31;
        x1 ^= x2;
        x1 = ROTL32(x1,r) + s[2*i];

        r = x1 & 31;
        x2 ^= x1;
        x2 = ROTL32(x2,r) + s[2*i + 1];
    }
    X1 = x1;
    X2 = x2;
}

void rc5_64_encrypt_2w(quint64 &X1, quint64 &X2, const quint64 *s) {
    quint64 x1 = X1 + s[0];
    quint64 x2 = X2 + s[1];
    int r;
    for ( int i = 1 ; i <= ROUNDS ; i++ ) {
        r = x2 & 63;
        x1 ^= x2;
        x1 = ROTL64(x1,r) + s[2*i];

        r = x1 & 63;
        x2 ^= x1;
        x2 = ROTL64(x2,r) + s[2*i + 1];
    }
    X1 = x1;
    X2 = x2;
}

void rc5_32_decrypt_2w(quint32 &X1, quint32 &X2, const quint32 *s) {
    quint32 x1 = X1;
    quint32 x2 = X2;
    int r;
    for ( int i = ROUNDS ; i > 0 ; i-- ) {
        r = x1 & 31;
        x2 -= s[2*i+1];
        x2 = ROTR32(x2,r) ^ x1;

        r = x2 & 31;
        x1 -= s[2*i];
        x1 = ROTR32(x1,r) ^ x2;
    }
    x2 -= s[1];
    x1 -= s[0];
    X2 = x2;
    X1 = x1;
}

void rc5_64_decrypt_2w(quint64 &X1, quint64 &X2, const quint64 *s) {
    quint64 x1 = X1;
    quint64 x2 = X2;
    int r;
    for ( int i = ROUNDS ; i > 0 ; i-- ) {
        r = x1 & 63;
        x2 -= s[2*i+1];
        x2 = ROTR64(x2,r) ^ x1;

        r = x2 & 63;
        x1 -= s[2*i];
        x1 = ROTR64(x1,r) ^ x2;
    }
    x2 -= s[1];
    x1 -= s[0];
    X2 = x2;
    X1 = x1;
}



void rc5_32_encrypt_8b(const uchar *plain8, uchar *cipher8, const quint32 *s) {
    quint32 X1 = qFromLittleEndian<quint32>(plain8);
    quint32 X2 = qFromLittleEndian<quint32>(plain8 + 4);
    rc5_32_encrypt_2w(X1, X2, s);
    qToLittleEndian(X1, cipher8);
    qToLittleEndian(X2, cipher8 + 4);
}

void rc5_64_encrypt_16b(const uchar *plain16, uchar *cipher16, const quint64 *s) {
    quint64 X1 = qFromLittleEndian<quint64>(plain16);
    quint64 X2 = qFromLittleEndian<quint64>(plain16 + 8);
    rc5_64_encrypt_2w(X1, X2, s);
    qToLittleEndian(X1, cipher16);
    qToLittleEndian(X2, cipher16 + 8);
}

void rc5_32_decrypt_8b(const uchar *cipher8, uchar *plain8, const quint32 *s) {
    quint32 X1 = qFromLittleEndian<quint32>(cipher8);
    quint32 X2 = qFromLittleEndian<quint32>(cipher8 + 4);
    rc5_32_decrypt_2w(X1, X2, s);
    qToLittleEndian(X1, plain8);
    qToLittleEndian(X2, plain8 + 4);
}

void rc5_64_decrypt_16b(const uchar *cipher16, uchar *plain16, const quint64 *s) {
    quint64 X1 = qFromLittleEndian<quint64>(cipher16);
    quint64 X2 = qFromLittleEndian<quint64>(cipher16 + 8);
    rc5_64_decrypt_2w(X1, X2, s);
    qToLittleEndian(X1, plain16);
    qToLittleEndian(X2, plain16 + 8);
}



}; //namespace
