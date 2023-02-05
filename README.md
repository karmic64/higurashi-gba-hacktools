# higurashi-gba-hacktools

Due to a lack of interest from me, consider this project on "infinite hiatus". I may be willing to start it up again, if:
* The staff room gets translated
* The image format ever gets figured out

If you are capable of and interested in fulfilling any of the above, please don't hesitate to contact me.

This repository may still be interesting to technically-minded people, as the reverse-engineering side of it is actually almost complete, bar the following:
* The image format was never figured out
* Some earlier games use `$0A`-`$0C` as their control codes instead of `$0D`-`$0F`
* The Higurashi command set appears to be the most "complete" set, and earlier games lack some of these commands. If you want to hack a different game, you need to keep this in mind.
* Some GBA-side commands weren't figured out, and their descriptions refer to exact memory addresses. These addresses are for Higurashi, and may differ per game. The official names on the PC-side compiler may help in figuring out their exact functions.
* Some PC-side commands weren't figured out. The effect commands will be especially troublesome, as they define special effects in a format that hasn't been figured out yet instead of mapping directly to script commands.

Additionally, the protection-killing tool is (as far as I know) 100% working. It has been confirmed to work with the following ports:
* Air
* Cross Channel
* Hajimete no Orusuban (doesn't have any emulator protection code)
* Higurashi no Naku Koro ni
* Kagetsu Touya
* Kimi ga Nozomu Eien
* Koibito Double ONE
* Suika (doesn't have any emulator protection code)
* To Heart 2
* Tsukihime

I will update the repository if a new port happens to be discovered that the tool does not work on. If you have a problem, contact me.

## Building

It is actually possible to build a semi-working translated Higurashi ROM with what you see here. Just keep in mind the following issues:
* Some text is completely untranslated, due to no direct equivalent existing in the 07th-Mod scripts. Some lines may appear to be "randomly" untranslated, because the 07th-Mod Japanese line has a few characters difference compared to the original.
* The 07th-Mod English text uses some control codes `<enclosed in these>`, which aren't cleaned out.
* Text rendering sucks, the characters are too wide, fixed-width, and there's no word wrapping. You can see in `ascfont.xcf` the font I wanted to implement.

Just for fun, I've included a BPS-format patch. It was created from the commonly-available, protected ROM (CRC32 `3C2F7D28`), but there's really no reason it shouldn't work on any dump of the game out there. This patch comes pre-cracked and you should not run the crack tool on it.

# Original readme

Eventually, this repository will contain tools to extract and rebuild the scripts of Inside-cap's Higurashi no Naku Koro ni GBA port (and possibly others), so we can all enjoy it in English. But for now, all there is is an incomplete technical reference and a protection cracking/ROM header repair tool. Please be patient!

## Thanks

I wish to thank the following people/groups for their (indirect) assistance in making this a possibility:
* Ryukishi07 - for making the game
* Inside-cap - for allowing on-the-go visual novel reading in a pre-phone world, and giving me a nice boredom-killer
* Unknown person(s) - for spreading their ROMs of several Inside-cap games
* "DJ" - for somehow figuring out how to remove the protection from the scripts
* [07th-Mod](https://07th-mod.com/) - for providing their source scripts which all the text was ripped from
* [Higurashi Archive](https://twitter.com/FurudeJinja) - for supplying the PC converter software
* [Nicolate](https://twitter.com/NicoIate) - for assuring me that there is at least one other person in the world who would actually be interested in this