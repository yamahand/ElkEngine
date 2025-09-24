#pragma once

namespace elk::logger {
    class LogMessage;
    
    class ILogSink{
        public:
            virtual ~ILogSink() = default;
            virtual void Write(const LogMessage& msg) = 0;
    };
}