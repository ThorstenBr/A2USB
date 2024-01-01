
#define NEWCONFIG_MAGIC       0x0001434E // "NC\x01\x00"
#define NEWCONFIG_EOF_MARKER  0x00464F45 // "EOF\x00"
#define CFGTOKEN_REVISION     0x00005652 // "RV\xXX\x00"

#define CFGTOKEN_MODE_VGA     0x0000564D // "MV\x00\x00" VGA
#define CFGTOKEN_MODE_PCPI    0x00005A4D // "MZ\x00\x00" PCPI Applicard
#define CFGTOKEN_MODE_SER     0x0000534D // "MS\x00\x00" Serial
#define CFGTOKEN_MODE_PAR     0x0000504D // "MP\x00\x00" Parallel
#define CFGTOKEN_MODE_SNES    0x0000474D // "MG\x00\x00" SNESMAX
#define CFGTOKEN_MODE_NET     0x0000454D // "ME\x00\x00" Ethernet
#define CFGTOKEN_MODE_FILE    0x0000464D // "MF\x00\x00" Filesystem

#define CFGTOKEN_HOST_AUTO    0x00004148 // "HA\x00\x00" Autodetect
#define CFGTOKEN_HOST_II      0x00003248 // "H2\x00\x00" II/II+
#define CFGTOKEN_HOST_IIE     0x00004548 // "HE\x00\x00" IIe
#define CFGTOKEN_HOST_IIGS    0x00004748 // "HG\x00\x00" IIgs
#define CFGTOKEN_HOST_PRAVETZ 0x00005048 // "HP\x00\x00" Pravetz
#define CFGTOKEN_HOST_BASIS   0x00004248 // "HB\x00\x00" Basis 108

#define CFGTOKEN_MUX_LOOP     0x00004C53 // "SL\x00\x00" Serial Loopback
#define CFGTOKEN_MUX_UART     0x00005353 // "SS\x00\x00" UART
#define CFGTOKEN_MUX_USB      0x00005553 // "SU\x00\x00" USB CDC
#define CFGTOKEN_MUX_WIFI     0x00005753 // "SW\x00\x00" WiFi Modem
#define CFGTOKEN_MUX_PRN      0x00005053 // "SP\x00\x00" WiFi Printer
#define CFGTOKEN_SER_BAUD     0x02004253 // "SB\x00\x01" Serial Baudrate Divisor

#define CFGTOKEN_USB_HOST     0x00004855 // "UH\x00\x00" USB CDC Host
#define CFGTOKEN_USB_GUEST    0x00004755 // "UG\x00\x00" USB CDC Guest Device
#define CFGTOKEN_USB_MIDI     0x00004D55 // "UM\x00\x00" USB MIDI Guest Device

#define CFGTOKEN_WIFI_AP      0x00004157 // "WA\x00\x00" WiFi AP
#define CFGTOKEN_WIFI_CL      0x00004357 // "WC\x00\x00" WiFi Client

#define CFGTOKEN_WIFI_SSID    0x00005357 // "WS\x00\xSS" WiFi SSID
#define CFGTOKEN_WIFI_PSK     0x00005057 // "WP\x00\xSS" WiFi PSK

#define CFGTOKEN_WIFI_IP      0x04004957 // "WI\x00\xSS" WiFi IP
#define CFGTOKEN_WIFI_NM      0x04004E57 // "WN\x00\xSS" WiFi Netmask

#define CFGTOKEN_JD_HOST      0x0000484A // "JH\x00\xSS" JetDirect Hostname
#define CFGTOKEN_JD_PORT      0x0200504A // "JP\x00\x02" JetDirect Port

#define CFGTOKEN_FONT_00      0x00004656 // "VF\xXX\x00" Custom default font

#define CFGTOKEN_MONO_00      0x00005056 // "VP\x00\x00" Full Color Video
#define CFGTOKEN_MONO_80      0x00805056 // "VP\x80\x00" B&W Video
#define CFGTOKEN_MONO_90      0x00905056 // "VP\x90\x00" B&W Inverse
#define CFGTOKEN_MONO_A0      0x00A05056 // "VP\xA0\x00" Amber
#define CFGTOKEN_MONO_B0      0x00B05056 // "VP\xB0\x00" Amber Inverse
#define CFGTOKEN_MONO_C0      0x00C05056 // "VP\xC0\x00" Green
#define CFGTOKEN_MONO_D0      0x00D05056 // "VP\xD0\x00" Green Inverse
#define CFGTOKEN_MONO_E0      0x00E05056 // "VP\xE0\x00" C64
#define CFGTOKEN_MONO_F0      0x00F05056 // "VP\xF0\x00" Custom

#define CFGTOKEN_TBCOLOR      0x00005456 // "VT\xXX\x00" Custom default TBCOLOR
#define CFGTOKEN_BORDER       0x00004256 // "VB\xXX\x00" Custom default BORDER

#define CFGTOKEN_VIDEO7       0x00003756 // "V7\xXX\x00" Video 7 Enable / Disable
#define CFGTOKEN_RGBCOLOR     0x00005043 // "CP\xXX\x04" RGB Palette Entry Override

#define CFGTOKEN_INTERP       0x00004956 // "VI\x0X\x00" RGB Interpolation
#define CFGTOKEN_GRILL        0x00004756 // "VG\x0X\x00" RGB Aperture Grill
