#pragma once

// settings.h
// 4/1/2014 jichi

#include "sakurakit/skglobal.h"
#include <QtCore/QObject>

class SettingsPrivate;
// Root object for all qobject
class Settings : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(Settings)
  SK_EXTEND_CLASS(Settings, QObject)
  SK_DECLARE_PRIVATE(SettingsPrivate)
public:
  static Self *instance();

  explicit Settings(QObject *parent = nullptr);
  ~Settings();

public:
  bool isEmbedDriverNeeded() const;
  bool isWindowDriverNeeded() const;

  bool isEmbeddedTextNeeded() const; // Whether send embedded text; placeholder that always returns true

  bool isWindowTranslationEnabled() const; // Whether translate text
  bool isWindowTranscodingEnabled() const; // Whether fix text encoding
  bool isWindowTextVisible() const; // Whether display text after the translation
  bool isEmbeddedScenarioVisible() const; // Whether disable scenario text
  bool isEmbeddedScenarioTranslationEnabled() const; // Whether translate scenario text
  bool isEmbeddedScenarioTranscodingEnabled() const; // Whether translate scenario text
  bool isEmbeddedNameVisible() const;
  bool isEmbeddedNameTextVisible() const;
  bool isEmbeddedNameTranslationEnabled() const;
  bool isEmbeddedNameTranscodingEnabled() const;
  bool isEmbeddedOtherVisible() const;
  bool isEmbeddedOtherTranslationEnabled() const;
  bool isEmbeddedOtherTranscodingEnabled() const;

  int embeddedTranslationWaitTime() const;

  QString gameEncoding() const;

public slots:
  void load(const QString &json);
  void disable();

  void setWindowTranslationEnabled(bool t);
  void setWindowTranscodingEnabled(bool t);
  void setWindowTextVisible(bool t);
  void setEmbeddedScenarioVisible(bool t);
  void setEmbeddedScenarioTranslationEnabled(bool t);
  void setEmbeddedScenarioTranscodingEnabled(bool t);
  void setEmbeddedNameVisible(bool t);
  void setEmbeddedNameTextVisible(bool t);
  void setEmbeddedNameTranslationEnabled(bool t);
  void setEmbeddedNameTranscodingEnabled(bool t);
  void setEmbeddedOtherVisible(bool t);
  void setEmbeddedOtherTranslationEnabled(bool t);
  void setEmbeddedOtherTranscodingEnabled(bool t);

  void setEmbeddedTranslationWaitTime(int v);

  void setGameEncoding(const QString &v);

signals:
  void loadFinished(); // emit after load() is invoked

  void windowTranslationEnabledChanged(bool t);
  void windowTranscodingEnabledChanged(bool t);
  void windowTextVisibleChanged(bool t);
  void embeddedScenarioVisibleChanged(bool t);
  void embeddedScenarioTranslationEnabledChanged(bool t);
  void embeddedScenarioTranscodingEnabledChanged(bool t);
  void embeddedNameVisibleChanged(bool t);
  void embeddedNameTextVisibleChanged(bool t);
  void embeddedNameTranslationEnabledChanged(bool t);
  void embeddedNameTranscodingEnabledChanged(bool t);
  void embeddedOtherVisibleChanged(bool t);
  void embeddedOtherTranslationEnabledChanged(bool t);
  void embeddedOtherTranscodingEnabledChanged(bool t);

  void embeddedTranslationWaitTimeChanged(int v);

  void gameEncodingChanged(QString v);
};

// EOF
