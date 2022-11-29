# JSON-Config

JSON-Config is a small Qt library that simplifies the usage of JSON in a configuration.

It does so by adding an extra layer, which operates between your code and the Qt classes like QJsonDocument. This layer consists of JsonTreeItem objects, which are organized as a tree. This tree stores all the information of the QJsonDocument with a few differences which make it easier to use and with less code repetition.

The tree structure allows you to access each value by reference, so you don't have to copy the QJsonDocument when you load the JSON file in order to be able to edit all the values, before saving it at logout or shutdown.

Thus, the overall objective with this library can be described as follows:

* You should be able to load a JSON-file into a tree object with just one or two commands.
* You should be able to save the tree object as a JSON-file with just one or two commands.
* Every value in the tree should be accessible like a variable, i.e. you can access and change it. At most you would have to declare what datatype it is.

## Access Values By Reference

When you load a JSON file via QJsonDocument, QJsonObject, QJsonArray and QJsonValue, it is somewhat tedious to change the values of an element in the tree dynamically. This is because (as of Qt 5.4) it is not possible to modify nested JSON objects.

The layer, created by JsonTreeItem objects allows you to load using QJsonDocument, to modify the data and then to create a new QJsonDocument that can be saved to a byte array or a file.

## Use Values Without Initialization

Consider that you have a newly created configuration in a `config.json` file, which is completely empty. In addition, consider the following code:

```c++
JsonTreeItem root;
root.loadFromFile("config.json");
bool showHints = root.value("General Settings", "Show Hints on Startup").toBool();
```

Since the file `config.json` is empty, you would normally expect an error. However, with the JsonTreeItem class, what you get instead, is that every time you tell it to access an item with a specific path, if it doesn't exist it is created. That means, if you run the code above and then save with

```c++
root.saveToFile("config.json");
```

what you get is a JSON file that looks like this:

```json
{
    "General Settings": {
        "Show Hints on Startup": null
    }
}
```

In that sense, every time you read something, you assume that the tree should contain this node. In that way you don't have to declare any variable before using it. It will contain a a QVariant set to null.

## Extended Data Types with ConfigItem

ConfigItem is a derived class from JsonTreeItem, which extends the functionality such that complex datatypes (so far QStringList, QList\<int>, QMap\<QString, QString\>) can be directly accessed. Here is an example

```c++
ConfigItem config;
config.loadFromFile("config.json");
QStringList &recentFiles = config.stringList("General Settings", "Recent Files");
```

Since `recentFiles` is an alias for a variable in the tree, every change made to this QStringList will be saved in the tree. Thus, if you do the following

```c++
recentFiles.insert(0, "last opened filename");
if (recentFiles.size() > 10)
    recentFiles.resize(10);
config.saveToFile("config.json");
```

the string "last opened filename" will be added to the QStringList and saved to `config.json`. If `config.json` was empty before, it would look like this:

```json
{
    "General Settings": {
        "Recent Files": [
            "last opened filename"
        ]
    }
}
```
