# Functions - Achievements
Related Oculus Platform documentation: [https://developer.oculus.com/documentation/native/ps-achievements/](https://developer.oculus.com/documentation/native/ps-achievements/)

------
## achievements_add_count
//// admonition | achievements_add_count(achievement_name : `String`, count : `int`)

Adds `count` to the achievement with the given name. The achievement must be of type COUNT.

**Returs:** A `GDOculusPlatformPromise` that will contain `Dictionary` with information about the achievement's state if fulfilled. The promise will error if the user is not entitled or if the check fails.

Example response:
``` json linenums="1"
{
    "name": "my_count_achievement",
    "just_unlocked": false
}
```

/// details | Example
    type: tip
``` gdscript linenums="1"
GDOculusPlatform.achievements_add_count("my_count_achievement", 5)\
.then(func(add_count_resp : Dictionary):
    if add_count_resp.just_unlocked:
        print("Achievement unlocked!")
)\
.error(func(add_count_err):
    print("Unable to update achievement: ", add_count_err)
)
```
///
////

## achievements_add_fields
//// admonition | achievements_add_fields(achievement_name : `String`, fields : `String`)

Unlocks fields of an achievement of type BITFIELD. The `fields` argument must be a `String` containing either "0" or "1" characters. Every "1" will unlock the field in the corresponding position. For example, if the bitfield is `10011` and you call this function with `fields = 00110`, the result will be `10111`.

**Returs:** A `GDOculusPlatformPromise` that will contain `Dictionary` with information about the achievement's state if fulfilled. The promise will error if the user is not entitled or if the check fails.

Example response:
``` json linenums="1"
{
    "name": "my_bitfield_achievement",
    "just_unlocked": true
}
```

/// details | Example
    type: tip
``` gdscript linenums="1"
GDOculusPlatform.achievements_add_fields("my_bitfield_achievement", "0011")\
.then(func(add_fields_resp : Dictionary):
    if add_fields_resp.just_unlocked:
        print("Achievement unlocked!")
)\
.error(func(add_fields_err):
    print("Unable to update achievement: ", add_fields_err)
)
```
///
////

## achievements_unlock
//// admonition | achievements_unlock(achievement_name : `String`)

Unlocks the achievement with the given `achievement_name`. The achievement can be of any type.

**Returs:** A `GDOculusPlatformPromise` that will contain `Dictionary` with information about the achievement's state if fulfilled. The promise will error if the user is not entitled or if the check fails.

Example response:
``` json linenums="1"
{
    "name": "my_simple_achievement",
    "just_unlocked": true
}
```

/// details | Example
    type: tip
``` gdscript linenums="1"
GDOculusPlatform.achievements_unlock("my_simple_achievement")\
.then(func(unlock_achievement_resp : Dictionary):
    if unlock_achievement_resp.just_unlocked:
        print("Achievement unlocked!")
)\
.error(func(unlock_achievement_err):
    print("Unable to update achievement: ", unlock_achievement_err)
)
```
///
////

## achievements_get_all_definitions
//// admonition | achievements_get_all_definitions()

Requests all achievement definitions of this app.

**Returs:** A `GDOculusPlatformPromise` that will contain  an `Array` of `Dictionaries` with information about all the achievements if fulfilled. The promise will error if the user is not entitled or if the check fails.

Example response:
``` json linenums="1"
[
    {
        "name": "my_achievement",
        "bitfield_length": 0,
        "target": 0,
        "type": "SIMPLE"
    },
    {
        "name": "my_count_achievement",
        "bitfield_length": 0,
        "target": 64,
        "type": "COUNT"
    },
    {
        "name": "my_bitfield_achievement",
        "bitfield_length": 8,
        "target": 5,
        "type": "BITFIELD"
    }
]
```

/// details | Example
    type: tip
``` gdscript linenums="1"
GDOculusPlatform.achievements_get_all_definitions()\
.then(func(achievements : Array):
    for achievement in achievements:
        print("Achievement: ", achievement.name)
)\
.error(func(achievements_err):
    print("Unable to get achievement definitions: ", achievements_err)
)
```
///
////

## achievements_get_all_progress
//// admonition | achievements_get_all_progress()

Requests all the progress of the achievements of this app.

**Returs:** A `GDOculusPlatformPromise` that will contain  an `Array` of `Dictionaries` with information about progress of all the achievements if fulfilled. The promise will error if the user is not entitled or if the check fails.

Example response:
``` json linenums="1"
[
    {
        "name": "my_achievement",
        "current_count": 0,
        "current_bitfield": "",
        "is_unlocked": true,
        "unlock_time": 1683990416
    },
    {
        "name": "my_count_achievement",
        "current_count": 12,
        "current_bitfield": "",
        "is_unlocked": false,
        "unlock_time": 0
    },
    {
        "name": "my_bitfield_achievement",
        "current_count": 0,
        "current_bitfield": "10011",
        "is_unlocked": false,
        "unlock_time": 0
    }
]
```

/// details | Example
    type: tip
``` gdscript linenums="1"
GDOculusPlatform.achievements_get_all_progress()\
.then(func(achievements_progress : Array):
    for achievement_progress in achievements_progress:
        print("Achievement name: ", achievement_progress.name)
        print("Unlocked? ", achievement_progress.is_unlocked)
)\
.error(func(achievements_progress_err):
    print("Unable to get achievements progress: ", achievements_progress_err)
)
```
///
////

## achievements_get_definitions_by_name
//// admonition | achievements_get_definitions_by_name(achievement_names : `Array`)

Requests achievement definitions by name of this app. The `achievement_names` argument must contain `String`s only.

**Returs:** A `GDOculusPlatformPromise` that will contain  an `Array` of `Dictionaries` with information about the requested achievements if fulfilled. The promise will error if the user is not entitled or if the check fails.

Example response:
``` json linenums="1"
[
    {
        "name": "my_achievement",
        "bitfield_length": 0,
        "target": 0,
        "type": "SIMPLE"
    },
    {
        "name": "my_count_achievement",
        "bitfield_length": 0,
        "target": 1500,
        "type": "COUNT"
    },
    {
        "name": "my_bitfield_achievement",
        "bitfield_length": 7,
        "target": 3,
        "type": "BITFIELD"
    }
]
```

/// details | Example
    type: tip
``` gdscript linenums="1"
var achievements : Array = ["my_achievement", "my_bitfield_achievement"]

GDOculusPlatform.achievements_get_definitions_by_name(achievements)\
.then(func(achievement_defs : Array):
    for achievement_def_ in achievement_defs:
        print("Achievement name: ", achievement_def.name)
)\
.error(func(achievement_defs_err):
    print("Unable to get achievement definitions by name: ", achievement_defs_err)
)
```
///
////

## achievements_get_progress_by_name
//// admonition | achievements_get_progress_by_name(achievement_names : `Array`)

Requests progress information of achievements by their name. The `achievement_names` argument must contain `String`s only.

**Returs:** A `GDOculusPlatformPromise` that will contain  an `Array` of `Dictionaries` with information about progress of the requested achievements if fulfilled. The promise will error if the user is not entitled or if the check fails.

Example response:
``` json linenums="1"
[
    {
        "name": "my_achievement",
        "current_count": 0,
        "current_bitfield": "",
        "is_unlocked": false,
        "unlock_time": 0
    },
    {
        "name": "my_count_achievement",
        "current_count": 1500,
        "current_bitfield": "",
        "is_unlocked": true,
        "unlock_time": 1683990516
    },
    {
        "name": "my_bitfield_achievement",
        "current_count": 0,
        "current_bitfield": "00101",
        "is_unlocked": false,
        "unlock_time": 0
    },
]
```

/// details | Example
    type: tip
``` gdscript linenums="1"
var achievements : Array = ["my_count_achievement"]

GDOculusPlatform.achievements_get_progress_by_name(achievements)\
.then(func(achievements_progress : Array):
    for achievement_progress in achievements_progress:
        print("Achievement name: ", achievement_progress.name)
        print("Unlock_time: ", achievement_progress.unlock_time)
)\
.error(func(achievements_progress_err):
    print("Unable to get achievements progress: ", achievements_progress_err)
)
```
///
////