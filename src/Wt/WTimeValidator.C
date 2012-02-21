
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WApplication"
#include "Wt/WTimeValidator"
#include "Wt/WLogger"
#include "Wt/WStringStream"
#include "Wt/WWebWidget"

#ifndef WT_DEBUG_JS
#include "js/WTimeValidator.min.js"
#endif

namespace Wt {

LOGGER("WTimeValidator");

WTimeValidator::WTimeValidator(WObject *parent)
  : WValidator(parent)
{
  setFormat("h:mm a");
}

WTimeValidator::WTimeValidator(const WTime& bottom, const WTime& top,
			       WObject *parent)
  : WValidator(parent),
    bottom_(bottom),
    top_(top)
{
  setFormat("h:mm a");
}

WTimeValidator::WTimeValidator(const WT_USTRING& format, WObject *parent)
  : WValidator(parent)
{
  setFormat(format);
}

WTimeValidator::WTimeValidator(const WT_USTRING& format,
			       const WTime& bottom, const WTime& top,
			       WObject *parent)
  : WValidator(parent),
    bottom_(bottom),
    top_(top)
{
  setFormat(format);
}

void WTimeValidator::setInvalidNotATimeText(const WString& text)
{
  notATimeText_ = text;
}

WString WTimeValidator::invalidNotATimeText() const
{
  if (!notATimeText_.empty()) {
    WString s = notATimeText_;
    s.arg(formats_[0]);
    return s;
  } else
    return WString::tr("Wt.WTimeValidator.WrongFormat").arg(formats_[0]);
}


void WTimeValidator::setFormat(const WT_USTRING& format)
{
  formats_.clear();
  formats_.push_back(format);
  repaint();
}

void WTimeValidator::setFormats(const std::vector<WT_USTRING>& formats)
{
  formats_ = formats;
  repaint();
}

void WTimeValidator::setBottom(const WTime& bottom)
{
  if (bottom_ != bottom) {
    bottom_ = bottom;
    repaint();
  }
}

void WTimeValidator::setTop(const WTime& top)
{
  if (top_ != top) {
    top_ = top;
    repaint();
  }
}

void WTimeValidator::setInvalidTooEarlyText(const WString& text)
{
  tooEarlyText_ = text;
  repaint();
}

WString WTimeValidator::invalidTooEarlyText() const
{
  if (!tooEarlyText_.empty()) {
    WString s = tooEarlyText_;
    s.arg(bottom_.toString(formats_[0])).arg(top_.toString(formats_[0]));
    return s;
  } else
    if (bottom_.isNull())
      return WString();
    else
      if (top_.isNull())
        return WString::tr("Wt.WTimeValidator.TimeTooEarly")
          .arg(bottom_.toString(formats_[0]));
      else
        return WString::tr("Wt.WTimeValidator.WrongTimeRange")
          .arg(bottom_.toString(formats_[0]))
          .arg(top_.toString(formats_[0]));
}

void WTimeValidator::setInvalidTooLateText(const WString& text)
{
  tooLateText_ = text;
  repaint();
}

WString WTimeValidator::invalidTooLateText() const
{
  if (!tooLateText_.empty()) {
    WString s = tooLateText_;
    s.arg(bottom_.toString(formats_[0])).arg(top_.toString(formats_[0]));
    return s;
  } else
    if (top_.isNull())
      return WString();
    else
      if (bottom_.isNull())
        return WString::tr("Wt.WTimeValidator.TimeTooLate")
          .arg(top_.toString(formats_[0]));
      else
        return WString::tr("Wt.WTimeValidator.WrongTimeRange")
          .arg(bottom_.toString(formats_[0]))
          .arg(top_.toString(formats_[0]));
}

#ifndef WT_DEPRECATED_3_0_0
WTime WTimeValidator::parse(const WString& input)
{
  return WTime::fromString(input, "h:mm a");
}
#endif // WT_DEPRECATED_3_0_0

WValidator::Result WTimeValidator::validate(const WT_USTRING& input) const
{
  if (input.empty())
    return WValidator::validate(input);

  for (unsigned i = 0; i < formats_.size(); ++i) {
    try {
      WTime d = WTime::fromString(input, formats_[i]);

      if (d.isValid()) {
	if (!bottom_.isNull())
	  if (d < bottom_)
	    return Result(Invalid, invalidTooEarlyText());

	if (!top_.isNull())
	  if (d > top_)
	    return Result(Invalid, invalidTooLateText());
    
	return Result(Valid);
      }
    } catch (std::exception& e) {
      LOG_WARN("validate(): " << e.what());
    }
  }

  return Result(Invalid, invalidNotATimeText());
}

void WTimeValidator::loadJavaScript(WApplication *app)
{
  LOAD_JAVASCRIPT(app, "js/WTimeValidator.js", "WTimeValidator", wtjs1);
}

std::string WTimeValidator::javaScriptValidate() const
{
  loadJavaScript(WApplication::instance());

  WStringStream js;

  js << "new " WT_CLASS ".WTimeValidator("
     << (isMandatory() ? "true" : "false") << ",[";

  for (unsigned i = 0; i < formats_.size(); ++i) {
    WTime::RegExpInfo r = WTime::formatToRegExp(formats_[i]);

    if (i != 0)
      js << ',';

    js << "{"
       << "regexp:" << WWebWidget::jsStringLiteral(r.regexp) << ','
       << "getHour:function(results){" << r.hourGetJS << ";},"
       << "getMinute:function(results){" << r.minuteGetJS << ";},"
       << "getSecond:function(results){" << r.secGetJS << ";}"
       << "getMSec:function(results){" << r.msecGetJS << ";}"
       << "}";
  }

  js << "],";

  if (!bottom_.isNull())
    js << "new Time("
       << bottom_.hour() << ',' << bottom_.minute()-1 << ',' << bottom_.second()
       << ")";
  else
    js << "null";

  js << ',';

  if (!top_.isNull())
    js << "new Time("
       << top_.hour() << ',' << top_.minute()-1 << ',' << top_.second()
       << ")";
  else
    js << "null";

  js << ',' << invalidBlankText().jsStringLiteral()
     << ',' << invalidNotATimeText().jsStringLiteral()
     << ',' << invalidTooEarlyText().jsStringLiteral()
     << ',' << invalidTooLateText().jsStringLiteral()
     << ");";

  return js.str();
}

#ifndef WT_TARGET_JAVA
void WTimeValidator::createExtConfig(std::ostream& config) const
{
/*
  config << ",format:"
	 << WWebWidget::jsStringLiteral(WTime::extFormat(formats_[0]), '\'');

  try {
    if (!bottom_.isNull())
      config << ",minValue:"
	     << WWebWidget::jsStringLiteral(bottom_.toString(formats_[0]));
    if (top_.isNull())
      config << ",maxValue:"
	     << WWebWidget::jsStringLiteral(top_.toString(formats_[0]));
  } catch (std::exception& e) {
    LOG_ERROR(e.what());
  }

  if (!tooEarlyText_.empty())
    config << ",minText:" << tooEarlyText_.jsStringLiteral();
  if (!tooLateText_.empty())
    config << ",maxText:" << tooLateText_.jsStringLiteral();
  if (!notATimeText_.empty())
    config << ",invalidText:" << notATimeText_.jsStringLiteral();

  WValidator::createExtConfig(config);
  */
}
#endif //WT_TARGET_JAVA

}
