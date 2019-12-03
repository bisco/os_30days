int mystrcmp(const char *s1, const char *s2) {
    while(*s1) {
       if(*s1 == *s2) {

       } else if (*s1 > *s2){
            return 1;
       } else {
            return -1;
       }
       s1++;
       s2++;
    }
    if(*s2 != 0) {
        return 1;
    }
    return 0;
}
