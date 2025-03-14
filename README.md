Overview
-------------

DSON - Ducky's Simple Object Notation: This file format is designed to represent hierarchical data using key-value pairs. It allows storing both simple values and nested groups, making it suitable for structured configuration, metadata, or serialization needs.
<br/>

Basic Concepts
-------------

- Key-Value Pairs
  - Each data entry consists of a key followed by an equals sign (=) and a value.
  - Values can be single values (wrapped in [ ]) or nested groups (wrapped in { }).
  - Multiple key-value pairs can appear on the same line
- Groups (Nested Structures)
  - A key followed by = and { starts a new nested group.
  - Groups can contain other key-value pairs or even nested groups.
  - A closing } marks the end of a group.
<br/>

Syntax Rules
-------------

- File Structure
  - Token based Structure. Each token separated by space ' '
  - A token can be either:
    - Equal Sing '='
    - Open / Close squarebBrackets '[', ']'
    - Open / Close curly braces '{', '}'
    - Literal: any sequence of character (without space - except a single character listed above)
- Key-Value Definition
  - Each key is a single Literal.
  - A key must be followed by = sign
  - A value can either be a Literal warpped in square brackets ([ ]) or a group wrapped with curly braces ({ })
  - A single line can hold multiple key-value pairs but each pair has to be finished within the line (except for groups)
  - Groups can be nested indefinitely.
- Comments
  - Any line starting with # sign will be skipped
<br/>

Why Tho?
-------------

Quack. Quack. Quack.
