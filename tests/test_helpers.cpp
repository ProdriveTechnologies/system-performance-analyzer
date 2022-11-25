#include "src/helpers/helper_functions.h"

#include <gtest/gtest.h>

// Demonstrate some basic assertions.
TEST(Helpers, SplitString)
{
  char delim = ';';
  std::vector<std::string> result = { "a", "bc", "d ", "e" };
  std::string text;
  for (size_t i = 0; i < result.size() - 1; i++)
  {
    auto elem = result.at(i);
    text += elem + delim;
  }
  text += result.back();

  // Expect equality.
  EXPECT_EQ(result, Helpers::Split(text, delim));
}

TEST(Helpers, IsNumber)
{
  // Expect equality.
  EXPECT_FALSE(Helpers::isNumber("false"));
  EXPECT_FALSE(Helpers::isNumber(""));
  EXPECT_FALSE(Helpers::isNumber("--"));
  EXPECT_FALSE(Helpers::isNumber("12g8"));
  EXPECT_FALSE(Helpers::isNumber("banana41"));
  EXPECT_TRUE(Helpers::isNumber("12"));
  EXPECT_TRUE(Helpers::isNumber("0123"));
}

TEST(Helpers, RemoveIntersection)
{
  std::vector<int> vec1{ 1, 2, 3, 4 };
  std::vector<int> vec2{ 4, 5, 6, 7 };
  std::vector<int> result1{ 1, 2, 3 };

  auto tempvec = vec1;
  Helpers::RemoveIntersection(tempvec, vec2);
  EXPECT_EQ(result1, tempvec);
  tempvec = vec1;
  Helpers::RemoveIntersection(tempvec, vec1);
  EXPECT_EQ(std::vector<int>{}, tempvec);
}

TEST(Helpers, CombineVectors)
{
  std::vector<int> vec1{ 1, 2, 3, 4 };
  std::vector<int> vec2{ 4, 5, 6, 7 };
  std::vector<int> result{ 1, 2, 3, 4, 4, 5, 6, 7 };
  std::vector<int> result2{ 4, 5, 6, 7, 1, 2, 3, 4 };

  auto combined = Helpers::CombineVectors(vec1, vec2);
  EXPECT_EQ(result, combined);
  combined = Helpers::CombineVectors(vec2, vec1);
  EXPECT_EQ(result2, combined);
}

TEST(Helpers, ReplaceString)
{
  std::string modified = "1 + 1 = 2";
  Helpers::replaceStr(modified, " ", ";");

  EXPECT_EQ(modified, "1;+;1;=;2");
  Helpers::replaceStr(modified, ";+;", "");
  EXPECT_EQ(modified, "11;=;2");

  Helpers::replaceStr(modified, modified, modified);
  EXPECT_EQ(modified, modified);

  Helpers::replaceStr(modified, modified, "");
  EXPECT_EQ(modified, "");
}

TEST(Helpers, DecimalsToInt)
{
  EXPECT_EQ(Helpers::DecimalsToInt("1315", 0), 0);
  EXPECT_EQ(Helpers::DecimalsToInt("1315", 1), 1);
  EXPECT_EQ(Helpers::DecimalsToInt("1315", 4), 1315);
  EXPECT_ANY_THROW(Helpers::DecimalsToInt("number", 2));
}