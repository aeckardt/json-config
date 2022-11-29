#ifndef TEST_CONFIGITEM_H
#define TEST_CONFIGITEM_H

#include <QDir>
#include <QFile>

#include <gtest/gtest.h>
#include <configitem.h>

TEST(ConfigItem, RemoveItem)
{
    ConfigItem config;

    QString objPath = "Components";
    QString key = "Search Active";

    // Test, if key exists
    EXPECT_FALSE(config.contains(objPath, key));

    config.value(objPath, key) = false;

    // Test, if key exists now
    EXPECT_TRUE(config.contains(objPath, key));

    config.removeItem(objPath, key);

    // Test, if key exists after deletion
    EXPECT_FALSE(config.contains(objPath, key));
}

TEST(ConfigItem, BooleanType)
{
    ConfigItem config;

    QString objPath1 = "Components/View/Navigation";
    QString objPath2 = "Components/View/Documents";
    QString key = "Visible";

    // Test, if keys exist
    EXPECT_FALSE(config.contains(objPath1, key));
    EXPECT_FALSE(config.contains(objPath2, key));

    // Assign Values to Keys
    config.value(objPath1, key) = true;
    config.value(objPath2, key) = false;

    // Test again, if the keys exists
    EXPECT_TRUE(config.contains(objPath1, key));
    EXPECT_TRUE(config.contains(objPath2, key));

    // Test, if the values are of type bool
    ASSERT_EQ(config.value(objPath1, key).type(), QVariant::Bool);
    ASSERT_EQ(config.value(objPath2, key).type(), QVariant::Bool);

    // Check the values of the bool variables
    EXPECT_TRUE(config.value(objPath1, key).toBool());
    EXPECT_FALSE(config.value(objPath2, key).toBool());
}

TEST(ConfigItem, SaveAndLoad)
{
    ConfigItem config;

    QString objPath = "Components";
    QString key = "Search filter";

    // Test, if key exists
    EXPECT_FALSE(config.contains(objPath, key));

    // Add a string list with the key
    QStringList &filterList = config.stringList(objPath, key);
    filterList = QStringList{"Capacitor", "100nF"};

    int listCount = filterList.size();
    EXPECT_GT(listCount, 0);

    // Verify that the string list was created correctly
    EXPECT_TRUE(config.contains(objPath, key));
    EXPECT_TRUE(config.stringList(objPath, key).size() == filterList.size());

    // Define path for temporary file to save
    QString configFilePath = "test.json";

    // Save configuration to file
    config.saveToFile(configFilePath);

    // Test, if the file was created
    QFile configFile(configFilePath);
    ASSERT_TRUE(configFile.exists());

    // Delete all existing nodes in the config tree
    config.clear();

    // Test, if the key was deleted
    EXPECT_FALSE(config.contains(objPath, key));

    // Load the configuration from the file
    config.loadFromFile(configFilePath);

    // Test, if the deleted key was found again
    EXPECT_TRUE(config.contains(objPath, key));
    QStringList &filterListNew = config.stringList(objPath, key);

    // Verify that the list was loaded correctly
    EXPECT_EQ(filterListNew.size(), listCount);
    EXPECT_TRUE(config.stringList(objPath, key).size() == filterListNew.size());
}

#endif // TEST_CONFIGITEM_H