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
  bool isEnabled() const;
  bool isEngineEnabled() const;
  bool isWindowTranslationEnabled() const;
  //QString language() const;

public slots:
  void load(const QString &json);
  void setEnabled(bool t);
  void setEngineEnabled(bool v);
  void setWindowTranslationEnabled(bool v);
  //void setLanguage(const QString &v);

signals:
  void enabledChanged(bool t);
  void engineEnabledChanged(bool);
  void windowTranslationEnabledChanged(bool);
  //void languageChanged(QString);
};

// EOF
