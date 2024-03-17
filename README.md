# `tab` - Put Music in Your Hands!

Tab is a tiny command-line utility to render tablatures for numerous music instruments.

![Screenshot](/screenshot.png)]

## Features

* Supports almost every tablature notation!
    * [Guitar tabs](https://en.wikipedia.org/wiki/Tablature#Guitar_tablature) for the following instruments:
        * Guitar
        * Ukulele
        * [Cigar Box Guitar](https://en.wikipedia.org/wiki/Cigar_box_guitar)
        * [Diddley Bow](https://en.wikipedia.org/wiki/Diddley_bow) (1-string and 2-string in different tunings)
    * [Harmonica tabs](https://en.wikibooks.org/wiki/Harmonica/Tablature)
        * Diatonic harmonica
        * Chromatic harmonica
    * [Klavarscribo](https://en.wikipedia.org/wiki/Klavarskribo)
        * 48-key Piano
        * 25-key Toy Piano
    * [Piano tabs](https://www.wikihow.com/Read-Piano-Tabs)
    * [Kalimba tabs](https://www.kalimbaclasses.com/kalimba-guides/how-to-read-number-tabs)
    * [Flutes](https://www.whistletabs.com/)
        * Recorder
        * Irish Tin Whistle
        * Native American Flute (4, 5 and 6 hole)
    * [Ocarinas](https://www.tabs-ocarina.com/)
        * Pendant Ocarina
    * [Trumpets](https://www.amromusic.com/trumpet-fingering-chart)
        * Trumpet in Bb
    * [Saxophones](https://www.amromusic.com/saxophone-fingering-chart)
        * Alto Sax in Eb
    * [Jianpu](https://en.wikipedia.org/wiki/Numbered_musical_notation)
        * Chinese Numeric Notation for any other instrument
* Great for self-taught musicians and amateurs, also kid-friendly.
* Allows to transpose music for the most convenient way of playing tabs.
* Uses visually appealing colored unicode output, when possible.
* Very tiny, written in C with no external dependencies, should work on every machine!

MIDI support is planned, and please contribute if your favourite instrument is not on the list!

## Usage

It's easy to build and install `tab` from sources:

```
$ git clone https://github.com/zserge/tab
$ cd tab
$ make
$ ./tab -i uke -x 5 examples/ode_to_joy.abc

    A│-------------0---│---0---------------│-------------------│-------------│-
    E│-2---2-----------│-----------2---0---│-----------0---2---│---2-0---0---│-
    C│-----------------│-------------------│---2---2-----------│-------------│-
    g│---------0-------│-------0-----------│-------------------│-------------│-
    w: Freu-de, schö-ner Göt-ter-funk-en, Toch-ter aus E-ly-si-um,
    ...
```

## Input format

Tab work well with [ABC notation](https://abcnotation.com/) but you may use a simplified text notation, too.

Tab operate over a range of 4 octaves from C3 to B6. This seems to be sufficient for most instruments that use tablature music notation.

Notes are written as letters ("CDEFGAB" for octave 4 and "cdefgab" for octave 5). Notes in octave 3 are followed by `,` and notes in octave `6` are followed by `'`. Technically, octaves can be lowered and raised even further, but most instrument won't be able to display such tabs correctly.

Like in ABC notation, notes may be prefixed by `^` to make them sharp and `_` to make them flat. You may alternatively write `#` after the note to make it sharp.

White space is preserved, and `|` is rendered as a bar separator. The rest of the ABC notation is ignored.

Lines that do not contain a musical notation are rendered verbatim as plain text.

## Example

```
Here are C notes in 4 octaves:
C, C c c'
And here is a E minor scale:
E F# G A | B c d e
```

Rendered for a Tin Whistle you may see something like this:

```
    Here are C notes in 4 octaves:
    x   x   ○   ○
    x   x   ●   ●
    x   x   ●   ●
    x   x   ○   ○
    x   x   ○   ○
    x   x   ○   ○
    x   x       +
    And here is a E minor scale:
    ●   ●   ●   ●   │   ●   ○   ○   ●
    ●   ●   ●   ●   │   ○   ●   ●   ●
    ●   ●   ●   ○   │   ○   ●   ●   ●
    ●   ●   ○   ○   │   ○   ○   ●   ●
    ●   ○   ○   ○   │   ○   ○   ●   ●
    ○   ○   ○   ○   │   ○   ○   ●   ○
                    │               +
```

In Numeric notation it would look like:

```
    Here are C notes in 4 octaves:
          .  :
    1  1  1  1
    *
    And here is a E minor scale:
                       .  .  .
    3  ♯4  5  6  |  7  1  2  3
```

And for Cigar Box Guitar it would become:

```
    Here are C notes in 4 octaves:
    g│-x-------5---17-
    D│-x--------------
    G│-x---5----------
    And here is a E minor scale:
    g│---------0---2---│---4---5---7---9-
    D│-2---4-----------│-----------------
    G│-----------------│-----------------
```

## Contributing

Contributions to Tab are welcome! Whether you want to report a bug, request a feature, or submit a pull request, please feel free to get involved.

Tab is open-source software licensed under the [MIT License](/LICENSE).
