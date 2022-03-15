#include <exception>
#include <string>


class PanguException : public std::exception
{
 public:

  explicit PanguException(const char * message) : msg(message) {}

  explicit PanguException(const std::string & message) : msg(message) {}

  virtual ~PanguException() throw() {}

  virtual const char * what() const throw() { return msg.c_str(); }

 protected:
  std::string msg;
};