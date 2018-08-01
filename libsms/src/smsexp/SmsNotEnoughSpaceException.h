#ifndef SMSNOTENOUGHSPACEEXCEPTION_H
#define SMSNOTENOUGHSPACEEXCEPTION_H


#include "exception/TException.h"
#include <string>

namespace Sms {


class SmsNotEnoughSpaceException : public BlackT::TException {
public:
  SmsNotEnoughSpaceException(const char* nameOfSourceFile__,
                   int lineNum__,
                   const std::string& source__,
                   const std::string& problem__);
  
  const char* what() const throw();
  
  std::string problem() const;
protected:

  std::string problem_;
  
};


};


#endif 
