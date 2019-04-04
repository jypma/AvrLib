
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
    template<> class Log<Loggers::Main>: public MessagesEnabled<STR("M")> {};
    template<> class Log<Loggers::ESP8266>: public MessagesEnabled<STR("E")> {};
    //template<> class Log<Loggers::RFM12>: public MessagesEnabled<STR("R")> {};
    template<> class Log<Loggers::RxState>: public MessagesEnabled<STR("Rx")> {};
    template<> class Log<Loggers::TxState>: public MessagesEnabled<STR("Tx")> {};
    //template<> class Log<Loggers::Ambient>: public MessagesEnabled<STR("Am")> {};
    //template<> class Log<Loggers::Scanner>: public MessagesEnabled<STR("S")> {};
    //template<> class Log<Loggers::PIR>: public MessagesEnabled<STR("PIR")> {};
    //template<> class Log<Loggers::Power>: public MessagesEnabled<STR("Power")> {};
    //template<> class Log<Loggers::TWI>: public MessagesEnabled<STR("TWI")> {};
    //template<> class Log<Loggers::Dallas>: public MessagesEnabled<STR("Dallas")> {};
    //template<> class Log<Loggers::Passive>: public MessagesEnabled<STR("Passive")> {};
    //template<> class Log<Loggers::RS232Tx>: public MessagesEnabled<STR("RS232Tx")>, public TimingEnabled {};
    //template<> class Log<Loggers::RS232Tx>: public MessagesDisabled, public TimingEnabled {};
    //template<> class Log<Loggers::PinChangeInterrupt>: public MessagesDisabled, public TimingEnabled {};
    //template<> class Log<Loggers::DHT11>: public MessagesEnabled<STR("DHT11")> {};
#else
    //template<> class Log<Loggers::VisonicDecoder>: public TimingEnabled, public MessagesEnabled<STR("Vison")> {};
    //template<> class Log<Loggers::Streams>: public MessagesEnabled<STR("Streams")> {};
    //template<> class Log<Loggers::Serial>: public MessagesEnabled<STR("Serial")> {};
    template<> class Log<Loggers::TxState>: public MessagesEnabled<STR("TxState")> {};
    template<> class Log<Loggers::RxState>: public MessagesEnabled<STR("RxState")> {};
    template<> class Log<Loggers::FrequencyCounter>: public MessagesEnabled<STR("FrequencyCounter")> {};
    template<> class Log<Loggers::TWI>: public MessagesEnabled<STR("TWI")> {};
    template<> class Log<Loggers::Timing>: public MessagesEnabled<STR("Timing")> {};
    template<> class Log<Loggers::RFM12>: public MessagesEnabled<STR("RFM12")> {};
    template<> class Log<Loggers::ESP8266>: public MessagesEnabled<STR("ESP8266")> {};
    template<> class Log<Loggers::DHT11>: public MessagesEnabled<STR("DHT11")> {};
    template<> class Log<Loggers::Main>: public MessagesEnabled<STR("Main")> {};
    template<> class Log<Loggers::Power>: public MessagesEnabled<STR("Power")> {};
#endif
}
