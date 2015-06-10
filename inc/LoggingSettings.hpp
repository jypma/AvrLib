
namespace Loggers {
    // All loggers must be listed here as empty classes
    class VisonicDecoder;
}

namespace Logging {
    // Enabled loggers go here, default is disabled.
    template<> class Log<Loggers::VisonicDecoder>: public TimingEnabled {};
}
