// FeedbackController.h - Rotte HTTP per invio e lettura feedback.

#ifndef PALESTRA_FEEDBACKCONTROLLER_H
#define PALESTRA_FEEDBACKCONTROLLER_H

#include "httplib.h"

void registraRotteFeedback(httplib::Server& server);

#endif
