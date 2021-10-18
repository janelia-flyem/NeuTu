#ifndef ZDATACHUNK_H
#define ZDATACHUNK_H


class ZDataChunk
{
public:
  ZDataChunk();

public:
  void setReady(bool ready) {
    m_isReady = ready;
  };
  bool isReady() const {
    return m_isReady;
  }

  bool isValid() const {
    return m_isValid;
  }
  void invalidate() {
    m_isValid = false;
  }

  bool updateNeeded() const;

protected:
  bool m_isReady = false;
  bool m_isValid = true;
};

#endif // ZDATACHUNK_H
