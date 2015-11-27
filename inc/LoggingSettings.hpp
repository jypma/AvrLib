
namespace Loggers {
    // All loggers must be listed here as empty classes
    class VisonicDecoder;
    class Streams;
    class Serial;
    class RFM12;
}

namespace Logging {
    // Enabled loggers go here, default is disabled.
#ifdef AVR
    //template<> class Log<Loggers::Streams>: public MessagesEnabled<STR("RFM12")> {};
#else
    template<> class Log<Loggers::VisonicDecoder>: public TimingEnabled {};
    template<> class Log<Loggers::Streams>: public MessagesEnabled<STR("Streams")> {};
    //template<> class Log<Loggers::Serial>: public MessagesEnabled<STR("Serial")> {};
    template<> class Log<Loggers::RFM12>: public MessagesEnabled<STR("RFM12")> {};
#endif
}
