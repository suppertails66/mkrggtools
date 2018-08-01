#include "smsexp/SmsNotEnoughSpaceException.h"

namespace Sms {


SmsNotEnoughSpaceException::SmsNotEnoughSpaceException(const char* nameOfSourceFile__,
                                   int lineNum__,
                                   const std::string& source__,
                                   const std::string& problem__)
  : TException(nameOfSourceFile__,
                   lineNum__,
                   source__),
    problem_(problem__) { };

const char* SmsNotEnoughSpaceException::what() const throw() {
  return problem_.c_str();
}

std::string SmsNotEnoughSpaceException::problem() const {
  return problem_;
}


}; 
