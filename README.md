# encodeingdetect

Plugin for AkelPad to detect encoding of text file using Mozilla [universalchardet](http://www-archive.mozilla.org/projects/intl/UniversalCharsetDetection.html)
plus: Japanese encoding detect (shift_jis/jis/euc-jp/utf-8)
Original author: [shoshinsha](https://akelpad.sourceforge.net/forum/viewtopic.php?t=1069)

## How to use

Latest AkelPad dev version
https://akelpad.sourceforge.net/files/AkelPad-x64.zip
https://akelpad.sourceforge.net/files/tools/AkelUpdater.zip

Download plugin
https://github.com/setsumi/encodeingdetect/releases
and place `EncodingDetect.dll` in `AkelFiles\Plugs\` subfolder.

function (for ContextMenu plugin)

```
SEPARATOR
"Encoding Detect"
{
"ISO2022/Japanese" Call("EncodingDetect::NonUniEncoding")
"nsUniversalDetector"
{
"Japanese" Call("EncodingDetect::UniJaEncodingDetect")
"Chinese simplified" Call("EncodingDetect::UniChineseSimEncodingDetect")
"Chinese traditional" Call("EncodingDetect::UniChineseTRAEncodingDetect")
"Chinese" Call("EncodingDetect::UniChineseEncodingDetect")
"Korean" Call("EncodingDetect::UniKoreaEncodingDetect")
"CJK" Call("EncodingDetect::UniCJKEncodingDetect")
"NonCJK" Call("EncodingDetect::UniNonCJKEncodingDetect")
"Auto" Call("EncodingDetect::UniEncodingDetect")
}
"Auto Load" +Call("EncodingDetect::Main")
"Setting" Call("EncodingDetect::Settings")
}
```