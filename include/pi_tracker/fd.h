#pragma once // захист від повторного включення одного header-а в той самий .cpp

#include <fcntl.h>

namespace pi_tracker
{

    // RAII-власник файлового дескриптора Linux
    class Fd
    {
    public:
        // Відкриває файл; якщо не вдалося, кидає std::system_error
        explicit Fd(const char *path, int flags = O_RDONLY);
        ~Fd();

        Fd(const Fd &) = delete;
        Fd &operator=(const Fd &) = delete;
        Fd(Fd &&other) noexcept;
        Fd &operator=(Fd &&other) noexcept;

        bool isValid() const { return m_fd >= 0; } // короткі методи можна лишити в header-і
        int get() const { return m_fd; }

    private:
        int m_fd{-1};
    };

} // namespace pi_tracker