
namespace Loggers {
    // All loggers must be listed here as empty classes
    class VisonicDecoder;
    class Streams;
    class Scanner;
    class Serial;
    class RFM12;
    class ESP8266;
    class DHT11;
    class Timing;
    class Dallas;
    class Main;
    class Passive;
    class TWI;
    class PinChangeInterrupt;
}

namespace Logging {
    // Enabled loggers go here, default is disabled.
#ifdef AVR
    /* If you enable any loggers for AVR, you must define a function as follows:
     *
     * namespace Logging {
     *   template <typename... types>
     *   void onMessage(types... args) {
     *     ...
     *   }
     * }
     *
     * or invoke the macro LOGGING_TO(var) with "var" being a USART TX pin, or fifo that is regularly emptied.
     */

    template<> class Log<Loggers::Timing>: public MessagesEnabled<STR("Timing")> {};
    template<> class Log<Loggers::Main>: public MessagesEnabled<STR("Main")> {};
    template<> class Log<Loggers::TWI>: public MessagesEnabled<STR("TWI")> {};
    //template<> class Log<Loggers::Dallas>: public MessagesEnabled<STR("Dallas")> {};
    //template<> class Log<Loggers::Passive>: public MessagesEnabled<STR("Passive")> {};
    //template<> class Log<Loggers::Serial>: public MessagesDisabled, public TimingEnabled {};
    //template<> class Log<Loggers::PinChangeInterrupt>: public MessagesDisabled, public TimingEnabled {};
    //template<> class Log<Loggers::DHT11>: public MessagesEnabled<STR("DHT11")> {};
#else
    //template<> class Log<Loggers::VisonicDecoder>: public TimingEnabled, public MessagesEnabled<STR("Vison")> {};
    //template<> class Log<Loggers::Streams>: public MessagesEnabled<STR("Streams")> {};
    //template<> class Log<Loggers::Scanner>: public MessagesEnabled<STR("Scanner")> {};
    //template<> class Log<Loggers::Serial>: public MessagesEnabled<STR("Serial")> {};
    template<> class Log<Loggers::TWI>: public MessagesEnabled<STR("TWI")> {};
    template<> class Log<Loggers::Timing>: public MessagesEnabled<STR("Timing")> {};
    template<> class Log<Loggers::RFM12>: public MessagesEnabled<STR("RFM12")> {};
    template<> class Log<Loggers::ESP8266>: public MessagesEnabled<STR("ESP8266")> {};
    template<> class Log<Loggers::DHT11>: public MessagesEnabled<STR("DHT11")> {};
    template<> class Log<Loggers::Main>: public MessagesEnabled<STR("Main")> {};
#endif
}
