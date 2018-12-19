#ifndef ZUNITTEST_H
#define ZUNITTEST_H


class ZUnitTest
{
public:
  ZUnitTest(int argc, char *argv[]);

  int run();

private:
  int m_argc;
  char **m_argv;
};

#endif // ZUNITTEST_H
