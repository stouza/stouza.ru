#include "sslkeys.h"

using namespace std;

const string& SSLKeys::getPrivateKey() const { return privateKey; }
const string& SSLKeys::getCertificate() const { return certificate; }

SSLKeys::SSLKeys() :

privateKey(reinterpret_cast<const char*>(privateKeyArray), sizeof(privateKeyArray)),
certificate(reinterpret_cast<const char*>(certificateArray), sizeof(certificateArray))

{ }

constexpr const unsigned char SSLKeys::privateKeyArray[];
constexpr const unsigned char SSLKeys::certificateArray[];

