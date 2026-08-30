#pragma once
#include <Arduino.h>
#include <SD.h>
#include <ArduinoJson.h>

// Shared helpers for reading JSON off the microSD card without duplicating
// every string into the ArduinoJson document (the board has no PSRAM, so
// this matters). readFileRaw() loads a file into a heap buffer that the
// caller owns; deserializeJson() is then called on that mutable buffer so
// ArduinoJson can store pointers into it ("zero-copy") instead of copying
// every string value. Copy out anything you need into String/std::vector
// before freeing the buffer.
namespace sdjson {

// Human-readable reason for the most recent readFileRaw()/loadInto()
// failure, so UI code can show *why* a file didn't load instead of a bare
// "not found" (the user has only been able to report back via screen
// photos, not a Serial monitor, so this needs to be visible on-screen too).
// A function-local static (rather than a namespace-scope variable) keeps
// this header safely includable from multiple .cpp files without needing
// a C++17 inline variable or a separate .cpp for the definition.
inline String &lastError() {
    static String s;
    return s;
}

// Reads a whole file into a newly allocated, NUL-terminated buffer.
// Returns nullptr on failure (see sdjson::lastError() for why). Caller must
// delete[] the buffer.
inline char *readFileRaw(const String &path, size_t &outLen) {
    File f = SD.open(path, FILE_READ);
    if (!f) {
        lastError() = "open() failed: " + path;
        Serial.println("[sdjson] " + lastError());
        return nullptr;
    }
    size_t size = f.size();
    char *buf = new (std::nothrow) char[size + 1];
    if (!buf) {
        lastError() = "out of memory (" + String((unsigned)(size + 1)) + " bytes) for: " + path;
        Serial.println("[sdjson] " + lastError());
        f.close();
        return nullptr;
    }
    size_t read = f.read(reinterpret_cast<uint8_t *>(buf), size);
    f.close();
    buf[read] = '\0';
    outLen = read;
    if (read != size) {
        lastError() = "short read on " + path + ": expected " + String((unsigned)size) +
                       " got " + String((unsigned)read);
        Serial.println("[sdjson] " + lastError());
    }
    return buf;
}

// Loads a JSON document from `path` into `doc`. Returns true on success.
// NOTE: doc may reference memory inside `buf` (zero-copy strings) - keep
// `buf` alive until you are done reading fields out of `doc`.
template <typename JsonDocT>
inline bool loadInto(const String &path, JsonDocT &doc, char *&buf) {
    size_t len = 0;
    buf = readFileRaw(path, len);
    if (!buf) return false;
    DeserializationError err = deserializeJson(doc, buf, len);
    if (err) {
        lastError() = "JSON parse error in " + path + ": " + err.c_str();
        Serial.println("[sdjson] " + lastError());
        delete[] buf;
        buf = nullptr;
        return false;
    }
    return true;
}

// Iterates a .jsonl (one compact JSON object per line) file, calling
// `cb(JsonObjectConst)` for each line. Used for the bestiary/spell/magic
// item indexes so the full list never has to sit in RAM as raw text at
// once - only the parsed line's small DOM does.
template <typename Cb>
inline bool forEachLine(const String &path, size_t lineDocCapacity, Cb cb) {
    File f = SD.open(path, FILE_READ);
    if (!f) return false;
    String line;
    line.reserve(160);
    while (f.available()) {
        char c = f.read();
        if (c == '\n') {
            if (line.length() > 0) {
                DynamicJsonDocument doc(lineDocCapacity);
                if (!deserializeJson(doc, line)) {
                    cb(doc.as<JsonObjectConst>());
                }
            }
            line = "";
        } else if (c != '\r') {
            line += c;
        }
    }
    if (line.length() > 0) {
        DynamicJsonDocument doc(lineDocCapacity);
        if (!deserializeJson(doc, line)) {
            cb(doc.as<JsonObjectConst>());
        }
    }
    f.close();
    return true;
}

} // namespace sdjson
