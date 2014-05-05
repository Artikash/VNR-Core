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
  bool isWindowTranslationEnabled() const; // Whether translate text
  bool isWindowTranscodingEnabled() const; // Whether fix text encoding
  bool isWindowTextVisible() const; // Whether display text after the translation
  bool isEmbeddedScenarioVisible() const; // Whether disable scenario text
  bool isEmbeddedScenarioTranslationEnabled() const; // Whether translate scenario text
  bool isEmbeddedNameVisible() const;
  bool isEmbeddedNameTranslationEnabled() const;
  bool isEmbeddedOtherVisible() const;
  bool isEmbeddedOtherTranslationEnabled() const;

public slots:
  void load(const QString &json);

  void setWindowTranslationEnabled(bool t);
  void setWindowTranscodingEnabled(bool t);
  void setWindowTextVisible(bool t);
  void setEmbeddedScenarioVisible(bool t);
  void setEmbeddedScenarioTranslationEnabled(bool t);
  void setEmbeddedNameVisible(bool t);
  void setEmbeddedNameTranslationEnabled(bool t);
  void setEmbeddedOtherVisible(bool t);

signals:
  void windowTranslationEnabledChanged(bool t);
  void windowTranscodingEnabledChanged(bool t);
  void windowTextVisibleChanged(bool t);
  void embeddedScenarioVisibleChanged(bool t);
  void embeddedScenarioTranslationEnabledChanged(bool t);
  void embeddedNameVisibleChanged(bool t);
  void embeddedNameTranslationEnabledChanged(bool t);
  void embeddedOtherVisibleChanged(bool t);
  void embeddedOtherTranslationEnabledChanged(bool t);
};

// EOF
