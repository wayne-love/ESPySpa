#include "SpaProperties.h"


inline boolean isNumber(String s) {
    if (s.isEmpty()) {
        return false;
    }
    for (u_int i = 0; i < s.length(); i++) {
        if ((!isDigit(s[i])) && !(s[i] == '-') && !(s[i]=='.')) {
            return false;
        }
    }
    return true;
}

boolean SpaProperties::update_SpaTime(String year, String month, String day, String hour, String minute, String second){

    tmElements_t tm;
    tm.Year=CalendarYrToTm(year.toInt());
    tm.Month=month.toInt();
    tm.Day=day.toInt();
    tm.Hour=hour.toInt();
    tm.Minute=minute.toInt();
    tm.Second=second.toInt();

    SpaTime.update_Value(makeTime(tm));

    return true;
}
