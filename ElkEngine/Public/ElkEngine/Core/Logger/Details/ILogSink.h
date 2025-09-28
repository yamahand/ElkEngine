#pragma once

namespace elk::logger {
    struct LogMessage;
    
    class ILogSink{
        public:
            virtual ~ILogSink() = default;
            virtual void Write(const LogMessage& msg) = 0;
    };
}