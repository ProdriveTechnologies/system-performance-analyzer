#include <gtest/gtest.h>

#include "src/helpers/helper_functions.h"

// Demonstrate some basic assertions.
TEST(Helpers, SplitString)
{
  char delim = ';';
  std::vector<std::string> result = {"a", "bc", "d ", "e"};
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
  EXPECT_TRUE(Helpers::isNumber("12"));
  EXPECT_TRUE(Helpers::isNumber("12g8"));
  EXPECT_TRUE(Helpers::isNumber("banana41"));
}

TEST(Helpers, RemoveIntersection)
{
  std::vector<int> vec1{1, 2, 3, 4};
  std::vector<int> vec2{4, 5, 6, 7};
  std::vector<int> result{1, 2, 3, 5, 6};

  auto tempvec = vec1;
  Helpers::RemoveIntersection(tempvec, vec2);
  EXPECT_EQ(result, tempvec);
  tempvec = vec2;
  Helpers::RemoveIntersection(tempvec, vec1);
  EXPECT_EQ(result, tempvec);
  tempvec = vec1;
  Helpers::RemoveIntersection(tempvec, vec1);
  EXPECT_EQ(std::vector<int>{}, tempvec);
}

TEST(Helpers, CombineVectors)
{
  std::vector<int> vec1{1, 2, 3, 4};
  std::vector<int> vec2{4, 5, 6, 7};
  std::vector<int> result{1, 2, 3, 4, 4, 5, 6};
  std::vector<int> result2{4, 4, 5, 6, 1, 2, 3, 4};

  auto combined = Helpers::CombineVectors(vec1, vec2);
  EXPECT_EQ(result, combined);
  combined = Helpers::CombineVectors(vec2, vec1);
  EXPECT_EQ(result2, combined);
}