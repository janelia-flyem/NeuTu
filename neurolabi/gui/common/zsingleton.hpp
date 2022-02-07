#ifndef ZSINGLETON_HPP
#define ZSINGLETON_HPP

template<typename T>
class ZSingleton
{
public:
  virtual ~ZSingleton() {}

  static T& GetInstance() {
    static T g;
    return g;
  }

protected:
  ZSingleton() {};
  ZSingleton(ZSingleton&) = delete;
};

#endif // ZSINGLETON_HPP
