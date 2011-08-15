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

/*
 *
 *  CHANGED
 *
 *
 */

/*
 *  S I M P L E   Q T  R C 5
 *
 *  This is a simple Qt library that implements symetric encryption using RC5
 *
 *  Design goals:
 *
 *   - Make it very simple to add encryption to a Qt Program
 *        (just add this header file and one cpp-file to your Qt project)
 *   - Only use Standard Qt functionality (as of Qt 4.5) to ensure portability
 *   - Object oriented frontend
 *   - Provide "strong" encryption
 *
 *  I know this is a bad idea because:
 *   - it is unwise to implement encryption yourself
 *   - other implementations are (much) faster
 *   - there are good C++ encryption available (boton, cryptcc, openssl)
 *   - RC5 is patented (dont use commercially in USA without obtaining a
 *       license from RSA - I dont provide such license)
 *
 *
 *  Algorithms:
 *    Currently the implementation supports two algoritms
 *      RC5-32/32/20  (32 bit words, 32 rounds, 20 byte/160bit key)
 *      RC5-64/32/20  (64 bit words, 32 rounds, 20 byte/160bit key)
 *    More algorithms/variants can be added if needed
 *
 *
 *
 *  About RC5:
 *    RC5 is a very simple encryption algorithm.
 *    RC5-XX/32/20 is very secure
 *        (not that the 32/64 bit versions use different word-sizes and
 *         are suitable for different CPUs. They should be be equally strong.)
 */


/*
 * Quickstart - encrypt data
 *
 * Set up key:
 *   QSharedPointer k(new Key(QString("My secret key")));
 * Create Encryptor
 *   Encryptor e(k, RC5_32_32_20, ModeCFB);
 * Encrypt first data
 *   Error er;
 *   QByteArray cipher;
 *   er = e.encrypt(mySecretByteArray, cipher, false);
 *   if ( er ) {
 *      // something went wrong
 * Encrypt more data
 *   er (e.encrypt(moreSecretData, cipher, true));
 *
 *
 *
 *
 * Quickstart - decrypt data
 *
 * Set up key:
 *   QSharedPointer k(new Key(QString("My secret key")));
 * Create DecryptorWizard (autodetects parameters)
 *   DecryptorWizard dw(k);
 * And a decryptor
 *   Decryptor d;
 * Decrypt first data
 *   Error er;
 *   QByteArray plain;
 *   er = dw.decrypt(myEncryptedSecretData, plain, d, false);
 *   if ( er ) {
 *      // something went wrong
 * Decrypt more data
 *   d->decrypt(moreEncryptedData, plain, true));
 *
 *
 *
 *
 *
 *
 */


 /*
 *
 *
 *  Usage details:
 *
 *  There are 3 layers accessible to the programmer
 *
 *  3. Feature layer (recommended)
 *      - message headers add features
 *         o automatically choose correct algorithm and mode
 *         o makes it possible to determine if a key is correct
 *
 *  2. Mode layer
 *      - CFB mode of operation suitable for encryption of streams
 *      - CBC mode of operation
 *     Handles padding and Initialization Vectors
 *
 *  1. Block layer
 *      - Encrypt or decrypt a word with a size given by the
 *        actual algorithm
 *
 *
 *
 *
 *
 *
 *  Implementation details:
 *    - little endian words are assumed (big endian machines
 *       work, with little performance penalty)
 *
 *
 *    - Feature layer adds a header to the message making it
 *      slightly larger. The format of the encrypte message is:
 *
 *        ALGORITHM:MODE:[OPTIONS:]:DATA
 *
 *      where ALGORITHM = RC5-32/32/20 or RC5-64/32/20
 *                 MODE = CFB, CBC
 *                 DATA = plaintext
 *
 *    - The benefit is that
 *       a) it is possible to verify that the correct key
 *          is used (otherwise the header doesnt decrypt)
 *       b) it is possible to autodetect algorithm, mode
 *          and key by trying different combinations
 *
 *
 *
 *
 */

#ifndef SIMPLEQTRC5_H
#define SIMPLEQTRC5_H

#include <QtGlobal>
#include <QByteArray>
//#include <QSharedPointer>
#include <QObject>

class QString;

namespace SimpleQtRC5 {

class Encryptor;
class Decryptor;
class LayerMode;
class CFB;
class CBC;

enum Algorithm {
    NoAlgorithm = 0,
    DetectAlgorithm,
    RC5_FAST_32_20,
    RC5_32_32_20,
    RC5_64_32_20
};

enum Mode {
    NoMode = 0,
    DetectMode,
    ModeCBC,
    ModeCFB
};

enum Checksum {
    NoChecksum = 0,
    DetectChecksum,
    ChecksumSoft,
    ChecksumHard
};

enum Error {
    NoError = 0,
    // ErrorNoKey,
    ErrorNoAlgorithm,
    ErrorNoMode,
    ErrorInvalidKey,
    ErrorNotEnoughData,
    ErrorModeNotImplemented,
    ErrorAlgorithmNotImplemented,
    ErrorChecksumNotImplemented,
    ErrorAlreadyError
};


enum State {
    StateReset = 0,
    StateOn,
    StateError
};


class Info {
public:
    static Algorithm fastRC5();
    static QString errorText(Error e);
};



class Key : public QObject {
    Q_OBJECT
public:
    Key();
    Key(const QByteArray &key);
    Key(const QString &key);
    ~Key();

    // not for use by end application
    void expandKey32();
    void expandKey64();

    // variables
    QByteArray key;
    quint32 *s32;
    quint64 *s64;
private:
    void expandKeyXX();
};


/*
 * About end and reset()
 *  - If you encrypt/decrypt a piece of data (ie a file) in one chunk
 *    make sure end=true. After this, you can use the same LayerMode
 *    object to encrypt/decrypt something else
 *  - If you encrypt/decrypt i piece of data (ie a file or a network
 *    conversation) in more than one chunk, make sure the last chunk
 *    only has end=true.
 *  - Call reset() only if you want to start over after an error
 *    (typically ErrorInvalidKey or ErrorNotEnoughData);
 *    as long as you use end=true, you never need to reset().
 */
class Encryptor : public QObject {
    Q_OBJECT
public:
    Encryptor(Key* k, Algorithm a, Mode m, Checksum c);
    ~Encryptor();   
    Error encrypt(const QByteArray &plain, QByteArray &cipher, bool end);
    void reset();
private:
    Key* key;
    Algorithm algorithm;
    Mode mode;
    Checksum checksum;
    State state;
    LayerMode *modex;
};


// will attempt all different combinations, and give you a
// Decryptor back to decrypt rest of data or more messages
// from the same source
class DecryptorWizardEntry;
class DecryptorWizard {
public:
    DecryptorWizard();
    DecryptorWizard(Key* k, Algorithm a = DetectAlgorithm, Mode m = DetectMode);
    ~DecryptorWizard();

    void addParameters(Key* k, Algorithm a = DetectAlgorithm, Mode m = DetectMode);

    Error decrypt(const QByteArray &cipher, QByteArray &plain, Decryptor* decryptor, bool end = false);
    Error decryptToEnd(const QByteArray &cipher, QByteArray &plain);
private:
    QList<DecryptorWizardEntry*> entries;
};



class Decryptor : public QObject {
    Q_OBJECT
public:
    Decryptor(){}
    Decryptor(Key* k, Algorithm a, Mode m);
    ~Decryptor();
    Error decrypt(const QByteArray &cipher, QByteArray &plain, bool end);
    void reset();
    Checksum getChecksumType();
private:
    Key* key;
    Algorithm algorithm;
    Mode mode;
    State state;
    Checksum checksum;
    LayerMode *modex;
};



class InitializationVector {
public:
    static QByteArray getVector8();
    static QByteArray getVector16();
    static void reserveVector(QByteArray rv);
private:
    static void initiate();
    void fixReserved(QByteArray &v);
    InitializationVector();
    ~InitializationVector();
    static InitializationVector *singleInstance;
    QList<QByteArray> reservedVectors;
};



/* *** Layer 2 : mode layer *** */

/*
 * A single LayerMode object can handle only one encrypt OR decrypt
 * at a time
 */
class LayerMode {
public:
    virtual QByteArray encrypt(const QByteArray plain, bool end) = 0;
    virtual QByteArray decrypt(const QByteArray cipher, bool end) = 0;
    virtual void reset() = 0;
    virtual ~LayerMode() {}
};

class CFB : public LayerMode {
public:
    CFB(Key* k, Algorithm a);
    ~CFB();
    QByteArray encrypt(const QByteArray plain, bool end = false);
    QByteArray decrypt(const QByteArray cipher, bool end = false);
    void reset();
private:
    QByteArray buffer;
    int bufferpos;
    Algorithm algorithm;
    Key* key;
};

class CBC : public LayerMode {
public:
    CBC(Key* k, Algorithm a);
    ~CBC();
    QByteArray encrypt(const QByteArray plain, bool end);
    QByteArray decrypt(const QByteArray cipher, bool end);
    void reset();
private:
    QByteArray buffer;
    QByteArray cbcBuffer;
    QByteArray padHostageBuffer;
    int worksize;
    Algorithm algorithm;
    Key* key;
};


/* *** Layer 1 : block layer - experts only *** */

// input replaced by output
void rc5_32_encrypt_2w(quint32 &X1, quint32 &X2, const quint32 *s);
void rc5_64_encrypt_2w(quint64 &X1, quint64 &X2, const quint64 *s);
void rc5_32_decrypt_2w(quint32 &X1, quint32 &X2, const quint32 *s);
void rc5_64_decrypt_2w(quint64 &X1, quint64 &X2, const quint64 *s);


void rc5_32_encrypt_8b(const uchar *plain8, uchar *cipher8, const quint32 *s);
void rc5_64_encrypt_16b(const uchar *plain16, uchar *cipher16, const quint64 *s);
void rc5_32_decrypt_8b(const uchar *cipher8, uchar *plain8, const quint32 *s);
void rc5_64_decrypt_16b(const uchar *cipher16, uchar *plain16, const quint64 *s);


} // namespace


#endif // SIMPLEQTRC5_H
