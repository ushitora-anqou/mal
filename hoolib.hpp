#pragma once
#ifndef ZARUWORKS_HOOLIB_HPP
#define ZARUWORKS_HOOLIB_HPP

#include <algorithm>
#include <array>
#include <cstring>
#include <fstream>
#include <memory>
#include <ostream>
#include <random>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace HooLib {

const double PI = 3.14159265358979323846, PI_2 = 1.57079632679489661923,
             PI_4 = 0.785398163397448309616;

inline double rad2deg(double rad) { return rad * 180.0 / PI; }
inline double deg2rad(double deg) { return deg * PI / 180.0; }

inline std::string createErrorMsg(const std::string& what, const char* file,
                                  int line)
{
    std::stringstream ss;
    ss << "(<" << file << "," << line << ">" << what << ")";
    return ss.str();
}

#define HOOLIB_ERROR(msg) HooLib::createErrorMsg((msg), __FILE__, __LINE__)
#define HOOLIB_THROW(msg)                            \
    {                                                \
        throw std::runtime_error(HOOLIB_ERROR(msg)); \
    };
#define HOOLIB_THROW_IF(ret, msg) \
    if ((ret)) {                  \
        HOOLIB_THROW((msg));      \
    }
#define HOOLIB_THROW_UNLESS(ret, msg) HOOLIB_THROW_IF(!(ret), msg);

#define unless(cond) if (!(cond))
#define until(cond) while (!(cond))

#define HOOLIB_RANGE(con) std::begin(con), std::end(con)

#define HOOLIB_DEBUG std::cerr << HOOLIB_ERROR("DEBUG") << std::endl;

namespace detail {
template <class T>
void swapImpl(T& left, T& right)
{
    using namespace std;
    swap(left, right);
}
}  // namespace detail

template <class T>
void swap(T& left, T& right)
{
    detail::swapImpl(left, right);
}

template <class Iterator>
int iter_swap(Iterator left, Iterator right)
{
    if (left == right) return 0;
    swap(*left, *right);
    return 1;
}

template <class T>
bool even(T val)
{
    return val % 2 == 0;
}

template <class T>
bool odd(T val)
{
    return val % 2 == 1;
}

inline bool equal(double x, double y)
{
    const static double EQUAL_ERROR = 0.000000001;
    return std::abs(x - y) < EQUAL_ERROR;
}

inline bool equal0(double x) { return equal(x, 0); }

inline double divd(double lhs, double rhs) { return lhs / rhs; }

template <class T>
constexpr const T& clamp(const T& v, const T& lo, const T& hi)
{
    return clamp(v, lo, hi, std::less<>());
}

template <class T, class Compare>
constexpr const T& clamp(const T& v, const T& lo, const T& hi, Compare comp)
{
    return assert(!comp(hi, lo)), comp(v, lo) ? lo : comp(hi, v) ? hi : v;
}

template <class T>
bool between(T l, T x, T r)
{
    return l < x && x < r;
}

template <class T>
bool betweenEq(T l, T x, T r)
{
    return l <= x && x <= r;
}

template <class InputIterator, class Callback>
void search(InputIterator first, InputIterator last, const std::regex& re,
            Callback callback)
{
    std::sregex_iterator it(first, last, re);
    std::sregex_iterator end;

    while (it != end) callback(*it++);
}

inline std::string fok(const std::string& str) { return str; }

template <class... Args>
std::string fok(const std::string& head, Args... args)
{
    return head + fok(args...);
}

template <class Iterator, class Callback>
std::string join(Iterator begin, Iterator end, const std::string& delim,
                 Callback cb)
{
    if (begin == end) return "";
    std::stringstream ss;
    for (auto it = begin;;) {
        ss << cb(*it++);
        if (it == end) break;
        ss << delim;
    }
    return ss.str();
}

template <class Iterator>
std::string join(Iterator begin, Iterator end, const std::string& delim)
{
    return join(begin, end, delim, [](auto&& item) { return item; });
}

template <class Head, class... Args>
Head max(Head head, Args... args)
{
    auto tmp = max(args...);
    return head < tmp ? tmp : head;
}

template <class Head, class Tail>
Head max(Head head, Tail tail)
{
    return head < tail ? tail : head;
}

template <class Head, class... Args>
Head min(Head head, Args... args)
{
    auto tmp = max(args...);
    return tmp < head ? tmp : head;
}

template <class Head, class Tail>
Head min(Head head, Tail tail)
{
    return tail < head ? tail : head;
}

template <class T>
std::string to_str(T&& t)
{
    std::stringstream ss;
    ss << t;
    return ss.str();
}

template <>
inline std::string to_str(std::string& str)
{
    return str;
}

template <>
inline std::string to_str(const std::string& str)
{
    return str;
}

// convert string to int
// if string has any invalid char, it throws exception
// errno will not be changed
inline int str2int(const std::string& str, int base = 0)
{
    auto errno_org = errno;
    int ret = 0;
    try {
        size_t idx;
        ret = stoi(str, &idx, base);
        HOOLIB_THROW_UNLESS(str.size() == idx,
                            "invalid argument: str should have only numbers");
    }
    catch (...) {
        errno = errno_org;
        throw;
    }
    errno = errno_org;
    return ret;
}

inline std::mt19937& getRandomEngine()
{
    static std::mt19937 engine = std::mt19937(std::random_device()());
    return engine;
}

// min <= x <= max
inline int randomInt(int min, int max)
{
    return std::uniform_int_distribution<>(min, max)(getRandomEngine());
}

// min <= x < sup
inline double randomFloat(double min, double sup)
{
    return std::uniform_real_distribution<>(min, sup)(getRandomEngine());
}

// return actual number of swaps
template <class RandomAccessIterator>
int shuffle(RandomAccessIterator first, RandomAccessIterator last)
{
    if (first == last) return 0;

    using distance_type =
        typename std::iterator_traits<RandomAccessIterator>::difference_type;
    using unsigned_type = typename std::make_unsigned<distance_type>::type;
    using distribute_type =
        typename std::uniform_int_distribution<unsigned_type>;

    auto g = getRandomEngine();
    distribute_type d;
    int ret = 0;
    for (auto it = first + 1; it != last; ++it) {
        auto itt = first + randomInt(0, it - first);
        if (it == itt) continue;
        ret++;
        iter_swap(it, itt);
    }
    return ret;
}

class Rect {
private:
    int x_, y_, w_, h_;

public:
    struct XYWH {
    };
    struct LTRB {
    };

    Rect() : x_(0), y_(0), w_(0), h_(0) {}
    Rect(int x, int y, int w, int h, XYWH) : x_(x), y_(y), w_(w), h_(h) {}
    Rect(int l, int t, int r, int b, LTRB) : x_(l), y_(t), w_(r - l), h_(b - t)
    {
    }

    int left() const { return x_; }
    int top() const { return y_; }
    int right() const { return x_ + w_; }
    int bottom() const { return y_ + h_; }
    int x() const { return x_; }
    int y() const { return y_; }
    int width() const { return w_; }
    int height() const { return h_; }
};

inline Rect XYWH(int x, int y, int w, int h)
{
    return Rect(x, y, w, h, Rect::XYWH());
}
inline Rect LTRB(int l, int t, int r, int b)
{
    return Rect(l, t, r, b, Rect::LTRB());
}

// multi-dimention array made of std::array
// thanks to https://www.ruche-home.net/boyaki/2013-12-28/Carray
template <typename T, std::size_t Size, std::size_t... Sizes>
struct multi_array_type {
    using type = std::array<typename multi_array_type<T, Sizes...>::type, Size>;
};
template <typename T, std::size_t Size>
struct multi_array_type<T, Size> {
    using type = std::array<T, Size>;
};
template <typename T, std::size_t Size, std::size_t... Sizes>
using multi_array = typename multi_array_type<T, Size, Sizes...>::type;

//

inline std::vector<std::string> splitStrByChars(const std::string& src,
                                                const std::string& delimChars)
{
    std::shared_ptr<char> data(new char[src.size() + 1],
                               std::default_delete<char[]>());
    std::vector<std::string> ret;

    std::strcpy(data.get(), src.c_str());

    char* p = std::strtok(data.get(), delimChars.c_str());
    while (p != nullptr) {
        ret.emplace_back(p);
        p = std::strtok(nullptr, delimChars.c_str());
    }

    return std::move(ret);
}

///

namespace Geometry {

template <class T>
struct Vec2 {
    using type = T;

    static Vec2<T> zero() { return Vec2<T>(0, 0); }

    T x, y;

    Vec2() {}
    Vec2(T x, T y) : x(x), y(y) {}

    T lengthSq() const { return x * x + y * y; }
    T length() const
    {
        using std::sqrt;
        return sqrt(lengthSq());
    }
    Vec2<T> norm() const
    {
        const T len = length();
        if (len > 0) return Vec2<T>(x / len, y / len);
        return Vec2(0, 0);
    }
};

template <class T>
bool operator==(const Vec2<T>& lhs, const Vec2<T>& rhs)
{
    return lhs.x == rhs.x && lhs.y == rhs.y;
}

template <class T>
bool operator!=(const Vec2<T>& lhs, const Vec2<T>& rhs)
{
    return !(lhs == rhs);
}

template <class T>
Vec2<T>& operator+=(Vec2<T>& lhs, const Vec2<T>& rhs)
{
    lhs.x += rhs.x;
    lhs.y += rhs.y;
    return lhs;
}

template <class T>
Vec2<T>& operator-=(Vec2<T>& lhs, const Vec2<T>& rhs)
{
    lhs.x -= rhs.x;
    lhs.y -= rhs.y;
    return lhs;
}

template <class T>
Vec2<T>& operator*=(Vec2<T>& lhs, T k)
{
    lhs.x *= k;
    lhs.y *= k;
    return lhs;
}

template <class T>
Vec2<T>& operator/=(Vec2<T>& lhs, T k)
{
    lhs.x /= k;
    lhs.y /= k;
    return lhs;
}

template <class T>
Vec2<T> operator+(const Vec2<T>& lhs, const Vec2<T>& rhs)
{
    Vec2<T> ret(lhs);
    ret += rhs;
    return ret;
}

template <class T>
Vec2<T> operator-(const Vec2<T>& lhs, const Vec2<T>& rhs)
{
    Vec2<T> ret(lhs);
    ret -= rhs;
    return ret;
}

template <class T>
Vec2<T> operator*(const Vec2<T>& lhs, T rhs)
{
    Vec2<T> ret(lhs);
    ret *= rhs;
    return ret;
}

template <class T>
Vec2<T> operator*(T lhs, const Vec2<T>& rhs)
{
    Vec2<T> ret(rhs);
    ret *= lhs;
    return ret;
}

template <class T>
Vec2<T> operator/(const Vec2<T>& lhs, T rhs)
{
    Vec2<T> ret(lhs);
    ret /= rhs;
    return ret;
}

template <class T>
Vec2<T> operator-(const Vec2<T>& src)
{
    return Vec2<T>(-src.x, -src.y);
}

template <class T>
std::ostream& operator<<(std::ostream& os, const Vec2<T>& v)
{
    os << "(" << v.x << "," << v.y << ")";
    return os;
}

template <class T>
double distance(const Vec2<T>& lhs, const Vec2<T>& rhs)
{
    return (lhs - rhs).length();
}

template <class T>
double distanceSq(const Vec2<T>& lhs, const Vec2<T>& rhs)
{
    return (lhs - rhs).lengthSq();
}

template <class T>
inline bool equal(const Vec2<T>& lhs, const Vec2<T>& rhs)
{
    return equal0(distanceSq(lhs, rhs));
}

template <class T>
double dot(const Vec2<T>& lhs, const Vec2<T>& rhs)
{
    return lhs.x * rhs.x + lhs.y * rhs.y;
}

template <class T>
double cross(const Vec2<T>& lhs, const Vec2<T>& rhs)
{
    return lhs.x * rhs.y - lhs.y * rhs.x;
}

template <class T>
bool parallel(const Vec2<T>& lhs, const Vec2<T>& rhs)
{
    return equal0(cross(lhs, rhs));
}

template <class T>
bool vertical(const Vec2<T>& lhs, const Vec2<T>& rhs)
{
    return equal0(dot(lhs, rhs));
}

template <class T>
bool sameSide(const Vec2<T>& base, const Vec2<T>& v1, const Vec2<T>& v2)
{
    return cross(base, v1) * cross(base, v2) > 0;
}

template <class T>
bool sharpAngle(const Vec2<T>& v1, const Vec2<T>& v2)
{
    return dot(v1, v2) > 0;
}

template <class T>
Vec2<T> rotate(const Vec2<T>& v, double angle)
{
    return Vec2<T>(v.x * std::cos(angle) - v.y * std::sin(angle),
                   v.x * std::sin(angle) + v.y * std::cos(angle));
}

using Vec2d = Vec2<double>;
using Point = Vec2d;

struct Line {
    Point p;
    Vec2d v;
};

struct Segment : public Line {
    Point from() const { return p; }
    Point to() const { return Point(p.x + v.x, p.y + v.y); }
    double length() const { return v.length(); }
    double lengthSq() const { return v.lengthSq(); }
    double left() const { return std::min(p.x, p.x + v.x); }
    double right() const { return std::max(p.x, p.x + v.x); }
    double top() const { return std::min(p.y, p.y + v.y); }
    double bottom() const { return std::max(p.y, p.y + v.y); }
};

struct Circle {
    Point p;
    double r;
};

inline Segment makeSegment(const Point& from, const Point& to)
{
    return Segment{from, to - from};
}

}  // namespace Geometry

namespace Operator {
template <class T>
std::vector<T>& operator+=(std::vector<T>& lhs, const std::vector<T>& rhs)
{
    lhs.reserve(lhs.size() + rhs.size());
    lhs.insert(lhs.end(), rhs.begin(), rhs.end());
    return lhs;
}

template <class T>
std::vector<T> operator+(std::vector<T> lhs, const std::vector<T>& rhs)
{
    lhs += rhs;
    return lhs;
}
}  // namespace Operator

template <class Iterator>
class Range {
    using iter_traits = std::iterator_traits<Iterator>;
    // using difference_type = typename iter_traits::difference_type;
    using reference = typename iter_traits::reference;

private:
    Iterator begin_, end_;

public:
    Range(Iterator begin, Iterator end) : begin_(begin), end_(end) {}

    Iterator begin() const { return begin_; }
    Iterator end() const { return end_; }

    size_t size() const { return std::distance(begin_, end_); }

    reference operator[](size_t index)
    {
        auto it = begin_;
        std::advance(it, index);
        return *it;
    }

    reference operator[](size_t index) const
    {
        auto it = begin_;
        std::advance(it, index);
        return *it;
    }
};

inline std::string cpp_escape_string(const std::string& src)
{
    std::stringstream ss;
    ss << "\"";
    for (char ch : src) {
        switch (ch) {
            case '\n':
                ss << "\\n";
                break;
            case '\t':
                ss << "\\t";
                break;
            case '\\':
                ss << "\\\\";
                break;
            case '\"':
                ss << "\\\"";
                break;
            default:
                ss << ch;
                break;
        }
    }
    ss << "\"";
    return ss.str();
}

inline std::string cpp_unescape_string(const std::string& src)
{
    bool backslashed = false;
    std::string ret;
    for (size_t i = 1; i < src.size() - 1; i++) {
        char ch = src[i];
        if (backslashed) {
            backslashed = false;
            switch (ch) {
                case 'n':
                    ret.push_back('\n');
                    break;
                case '\\':
                    ret.push_back('\\');
                    break;
                case '"':
                    ret.push_back('"');
                    break;
                case 't':
                    ret.push_back('\t');
                    break;
            }
            continue;
        }

        if (ch == '\\') {
            backslashed = true;
            continue;
        }

        ret.push_back(ch);
    }
    return ret;
}

inline std::string getline_all(std::istream& is)
{
    std::stringstream ss;
    std::string input;
    while (std::getline(is, input)) ss << input << std::endl;
    return ss.str();
}

inline std::string read_file_all(const std::string& filename)
{
    std::ifstream ifs(filename);
    std::stringstream ss;
    ss << ifs.rdbuf();
    return ss.str();
}

}  // namespace HooLib

#endif
