/*
    《程序设计实习》魔兽世界大作业 4
    创建者  : weng_weijie
    日期    : 2023.4.2
*/

#include <iostream>
#include <algorithm>
#include <cstring>
#include <cassert>
#include <vector>
#include <array>
#include <string>

/* 游戏：整个游戏的命名空间 */
namespace WoW {

/* 游戏：控制器 */
class GameEngine;

/* 游戏：配置数据 */
struct GameConfig {
    int life_elements;
    int city_num;
    int arrow_attack;
    int loyalty_dec;
    int stop_time;
    std::array<int, 5> initial_hp;
    std::array<int, 5> initial_attack;
    std::vector<int> summon_order[2];
};

/* 阵营：CAMP_RED = 0, CAMP_BLUE = 1*/
enum Camp {
    CAMP_RED    = 0, 
    CAMP_BLUE   = 1,
};

const Camp CAMPS[] = {CAMP_RED, CAMP_BLUE};

/* 阵营：获取另一方阵营 */
Camp rival(Camp c) {return Camp(int(c) ^ 1); }
/* 阵营：获取阵营名字 */
std::string camp_name(Camp c) {return c == CAMP_RED ? "red" : "blue"; }

enum WarriorType {
    WARRIOR_DRAGON  = 0,
    WARRIOR_NINJA   = 1,
    WARRIOR_ICEMAN  = 2,
    WARRIOR_LION    = 3,
    WARRIOR_WOLF    = 4,
};

enum WeaponType {
    WEAPON_SWORD    = 0,
    WEAPON_BOMB     = 1,
    WEAPON_ARROW    = 2,
};

/* 伤害：伤害类型 */
enum DamageType {
    /* 普通攻击伤害 */
    DAMAGE_SWORD        = 0,
    /* Bomb 伤害*/
    DAMAGE_BOMB         = 1,
    /* Arrow 伤害 */
    DAMAGE_ARROW        = 2,
    /* Iceman 行走伤害 */
    DAMAGE_ICEMAN       = 3,
    /* 恢复血量 */
    DAMAGE_HEAL         = 4,
};

/* 事件-输出格式 */
const struct EType {int id; std::string fmt; }   
    EVENT_BORN        = { 1, "%s born"},
    EVENT_BORN_DRAGON = { 1, "%s born\nIts morale is %.2f"},
    EVENT_BORN_LION   = { 1, "%s born\nIts loyalty is %d"},
    EVENT_ESCAPE      = { 2, "%s ran away"},
    EVENT_MARCH       = { 3, "%s marched to %s with %d elements and force %d"},
    EVENT_ARROW       = { 4, "%s shot"},
    EVENT_ARROW_KILL  = { 4, "%s shot and killed %s"},
    EVENT_BOMB        = { 5, "%s used a bomb and killed %s"},
    EVENT_ATTACK      = { 6, "%s attacked %s in %s with %d elements and force %d"},
    EVENT_FIGHT_BACK  = { 7, "%s fought back against %s in %s"},
    EVENT_DEAD        = { 8, "%s was killed in %s"},
    EVENT_CHEER       = { 9, "%s yelled in %s"},
    EVENT_EARN        = {10, "%s earned %d elements for his headquarter"},
    EVENT_FLAG        = {11, "%s flag raised in %s"},
    EVENT_ARRIVED     = {12, "%s reached %s with %d elements and force %d"},
    EVENT_TAKEN       = {13, "%s was taken"},
    EVENT_REPORT_ELEM = {14, "%d elements in %s"},
    EVENT_REPORT_WEAP = {15, "%s has %s"};

/* 伤害 */
struct Damage {
    /* 伤害：伤害类型*/
    DamageType type;
    /* 伤害：伤害数值 */
    int dmg;
};

/* 旗帜类型 */
enum FlagType {
    FLAG_NONE   = -1,
    FLAG_RED    = 0,
    FLAG_BLUE   = 1,
};

enum BattleType {
    BATTLE_NONE = -2,
    BATTLE_DRAW = -1,
    BATTLE_RED  = 0,
    BATTLE_BLUE = 1,
};

/* 工具：游戏相关常用工具的命名空间 */
namespace utils {

/* 工具：生成格式化字符串 */
template <typename ...Args>
std::string format(const std::string& fmt, Args && ...args)
{
    auto size = std::snprintf(nullptr, 0, fmt.c_str(), std::forward<Args>(args)...);
    std::string output(size + 1, '\0');
    std::sprintf(&output[0], fmt.c_str(), std::forward<Args>(args)...);
    output.resize(size);
    return output;
}

/* 计时器 */
class Timer {
public:
    int hour, minute;
    /* 计时器：以 hhh:mm 格式获取当前时间 */
    std::string time_str() const {return format("%03d:%02d", hour, minute); }
    /* 计时器：以分钟数获取当前时间 */
    int time() const {return hour * 60 + minute; }
    /* 计时器：比较时间前后 */
    friend bool operator < (Timer a, Timer b) {
        return a.hour != b.hour ? a.hour < b.hour : a.minute < b.minute;
    }
    friend bool operator == (Timer a, Timer b) {
        return a.hour == b.hour && a.minute == b.minute;
    }
    friend bool operator != (Timer a, Timer b) {
        return a.hour != b.hour || a.minute != b.minute;
    }
};

/* 事件 */
class Event {
private:
    /* 事件：发生时间 */
    Timer time;
    /* 事件：类型 */
    int type;
    /* 事件：阵营编号 */
    int camp_id;
    /* 事件：城市编号 */
    int city_id;
    /* 事件：具体信息 */
    std::string info;

public:
    /* 事件：构造函数中传入各种信息用于排序 */
    Event(Timer time, int type, int camp_id, int city_id, std::string info):
        time(time), type(type), camp_id(camp_id), city_id(city_id), info(info) {}
    /* 事件：比较事件的输出先后顺序 */
    friend bool operator < (Event a, Event b) {
        /* 按照 (时间, 位置, 类型, 阵营) 进行排序 */
        if (a.time != b.time) return a.time < b.time;
        if (a.type == 15 && b.type == 15) {
            /* 注意：武士报告的顺序与其他时间输出的顺序不同 (阵营, 位置) */
            if (a.camp_id != b.camp_id) return a.camp_id < b.camp_id;
            if (a.city_id != b.city_id) return a.city_id < b.city_id;
            return 0;
        }
        if (a.city_id != b.city_id) return a.city_id < b.city_id;
        if (a.type != b.type) return a.type < b.type;
        if (a.camp_id != b.camp_id) return a.camp_id < b.camp_id;
        return 0;
    }
    /* 事件：输出 */
    std::string output() const {return time.time_str() + " " + info; }
};

}

/* 武士：基类 */
class WarriorBase;
/* 武士：Dragon */
class WarriorDragon;
/* 武士：Ninja */
class WarriorNinja;
/* 武士：Iceman */
class WarriorIceman;
/* 武士：Wolf */
class WarriorWolf;

/* 武器：基类 */
class WeaponBase;
/* 武器：Sword */
class WeaponSword;
/* 武器：Bomb */
class WeaponBomb;
/* 武器：Arrow */
class WeaponArrow;
/* 武器：武器列表 */
class WeaponList;

/* 城市：基类*/
class City;
/* 城市：司令部 */
class HeadQuarter;

class GameEngine final {
private:
    /* 游戏：游戏实例（只允许存在一个） */
    static GameEngine *instance;

    /* 游戏：当前时间 */
    utils::Timer now_time;

    /* 游戏：发生过的事件 */
    std::vector<utils::Event> events;

    /* 游戏：城市列表 */
    std::vector<City*> cities;

    HeadQuarter *hq[2];

    /* 游戏：配置数据 */
    GameConfig config;

    GameEngine() {}
    ~GameEngine();

    void initialize();

public:
    /* 游戏：获取游戏句柄 */
    static GameEngine* get_handle() {
        if (instance == nullptr)
            instance = new GameEngine();
        return instance;
    }

    /* 游戏：获取配置数据 */
    static GameConfig get_config() {return instance->config; }

    /* 游戏：发送事件 */
    static void log(utils::Event e) {instance->events.emplace_back(e); }

    template<typename ...Args>
    static void log(const Camp camp_id, const City* city, const EType event, Args && ...params);

    /* 游戏：获取事件 */
    static utils::Timer time() {return instance->now_time; }

    /* 游戏：载入配置 */
    static void load_settings(GameConfig conf) {
        auto game = get_handle();
        game->config = conf;
    }

    /* 游戏：开始运行 */
    static void start();

    /* 游戏：输出结果 */
    static void output() {
        std::sort(instance->events.begin(), instance->events.end());
        for (utils::Event e : instance->events)
            std::cout << e.output() << std::endl;

    }

    /* 游戏：结束运行 */
    static void stop() {
        if (instance == nullptr)
            return;
        delete instance;
        instance = nullptr;
    }
};

GameEngine* GameEngine::instance = nullptr;

class WarriorBase {
protected:
    /* 武士：攻击力*/
    int attack;
    /* 武士：血量 */
    int hp;
    /* 武士：编号 */
    int id;
    /* 武士：所在阵营 */
    Camp camp;
    /* 武士：所持武器列表 */
    WeaponList *weapons;
    /* 武士：当前所在城市 */
    City *now_at;
    /* 武士：所在司令部 */
    HeadQuarter *head_at;

public:
    /* 武士：被司令部生成 */
    WarriorBase(HeadQuarter*);
    virtual ~WarriorBase();

    /* 武士：获取武士类型 */
    virtual int get_type() const = 0;
    /* 武士：获取武士名字 */
    virtual std::string get_name() const = 0;
    /* 武士：初始化信息 */
    virtual void initialize();

    /* 武士：获取武士信息 */
    virtual std::string info() const {
        return utils::format("%s %s %d", camp_name(camp).c_str(), get_name().c_str(), id);
    }
    HeadQuarter* get_head() const {return head_at; }
    Camp get_camp() const {return camp; }
    int get_attack() const {return attack; }
    WeaponList* get_weapons() const {return weapons; }
    virtual int real_attack(bool) const;
    int get_hp() const {return hp; }
    WarriorBase* get_rival() const;

    /* 武士：事件-出生*/
    virtual void on_born();

    /* 武士：静默销毁自己 */
    virtual void destroy();

    /* 武士：事件-时间改变（此处发生无序事件） */
    virtual void on_time_changing(utils::Timer);

    /* 武士：前方城市 */
    virtual City* forward() const;
    /* 武士：事件-前进 */
    virtual void on_move_forward();

    /* 武士：事件-检查使用 Arrow */
    virtual bool on_check_arrow();

    /* 武士：事件-使用 Arrow */
    virtual void on_use_arrow(WarriorBase*);

    /* 武士：事件-检查使用 Bomb */
    virtual bool on_check_bomb();

    /* 武士：事件-使用 Bomb */
    virtual void on_use_bomb(WarriorBase*);

    /* 武士：事件-受到攻击（返回：是否死亡）*/
    virtual bool on_damaged(WarriorBase*, Damage);

    /* 武士：事件-主动攻击 */
    virtual void on_attack(WarriorBase*);

    /* 武士：事件-反击 */
    virtual void on_fight_back(WarriorBase*);

    /* 武士：事件-胜利（主动攻击）*/
    virtual void on_win(WarriorBase*, bool);

    virtual void on_draw(WarriorBase*, bool) {}

    /* 武士：事件-战死 */
    virtual void on_dead(WarriorBase*);

    /* 武士：事件-抢夺城市中生命元 */
    virtual void on_loot_elements();

    /* 武士：报告武器情况 */
    virtual void report() const;
};

class WeaponBase {
protected:
    /* 武器：耐久度 */
    int damage;
public:
    /* 武器：获取武器类型 */
    virtual int get_type() const = 0;
    /* 武器：获取武器名字 */
    virtual std::string get_name() const = 0;
    /* 武器：获取武器信息 */
    virtual std::string info() const {
        return utils::format("%s(%d)", get_name().c_str(), damage);
    }

    int get_damage() const {return damage; }
    /* 武器：被使用 */
    virtual void on_used() {--damage; }
    /* 武器：已使用完毕 */
    virtual bool is_damaged() {return damage == 0; }
};

class WeaponList final {
private:
    WeaponBase* weapon[3];
public:
    WeaponList() {for (auto &w : weapon) w = nullptr; }
    ~WeaponList() {for (auto w : weapon) if (w != nullptr) delete w; }

    /* 武器列表：获取某个编号武器 */
    WeaponBase* get(int id) {return weapon[id]; }

    bool insert(WeaponBase *w) {
        if (w == nullptr) return false;
        if (weapon[w->get_type()] != nullptr) return false;
        // assert(weapon[w->get_type()] == nullptr);
        weapon[w->get_type()] = w;
        return true;
    }

    /* 武器列表：事件-将另一列表合并过来 */
    void on_merge(WeaponList *w) {
        for (int i = 0; i < 3; ++i) {
            if (!insert(w->weapon[i])) {
                delete w->weapon[i];
            }
            w->weapon[i] = nullptr;
        }
    }

    /* 武器列表：事件-使用某个编号武器 */
    void on_used(int id) {
        if (weapon[id] == nullptr) return;
        weapon[id]->on_used();
        if (weapon[id]->is_damaged()) {
            delete weapon[id];
            weapon[id] = nullptr;
        }
    }

    /* 武器列表：报告武器情况 */
    std::string report() {
        std::string s;
        for (int i : {2, 1, 0}) {
            WeaponBase *w = weapon[i];
            if (w != nullptr)
                s += (s == "" ? "" : ",") + w->info();
        }
        if (s == "")
            return "no weapon";
        return s;
    }
};

class City {
protected:
    /* 城市：编号 */
    int id;
    /* 城市：生命元数量 */
    int live_elements;
    /* 城市：旗帜 */
    FlagType flag;
    /* 城市：在此城市的武士 */
    WarriorBase *warrior[2];
    /* 城市：西边、东边的相邻城市 */
    City *w, *e;
    /* 城市：上回合获胜阵营 */
    BattleType last_battle, history_battle;
public:
    City(int id) : id(id) {
        live_elements = 0;
        flag = FLAG_NONE;
        warrior[CAMP_RED] = warrior[CAMP_BLUE] = nullptr;
        last_battle = history_battle = BATTLE_NONE;
    }

    void set_neighbors(City *west, City *east) {
        w = west, e = east;
    }

    /* 城市：获取编号 */
    int get_id() const {return id; }
    /* 城市：获取详细信息 */
    virtual std::string info() const {return utils::format("city %d", id); }
    /* 城市：获取生命元数量 */
    int get_elem() const {return live_elements; }

    /* 城市：是否为司令部 */
    virtual bool is_headquarter() const {return false; }
    /* 城市：获得西边城市 */
    virtual City* west() const {return w; }
    /* 城市：获得东边城市 */
    virtual City* east() const {return e; }

    /* 城市：获取某阵营武士 */
    virtual WarriorBase* get_warrior(Camp c) const {return warrior[int(c)]; }
    WarriorBase* get_winner() const {
        if (last_battle == BATTLE_DRAW || last_battle == BATTLE_NONE)
            return nullptr;
        return get_warrior(Camp(last_battle));
    }
    /* 城市：删除某阵营武士 */
    virtual void remove(Camp c)  {warrior[int(c)] = nullptr; }
    /* 城市：清除城市内尸体 */
    virtual void clear();

    /* 城市：事件-时间改变（此处发生无序事件） */
    virtual void on_time_changing(utils::Timer);

    virtual Camp which_active() const;

    /* 城市：进行战斗并得到结果 */
    virtual BattleType battle(WarriorBase*, WarriorBase*);

    /* 城市：事件-发生战斗 */
    virtual void on_battle();

    /* 城市：事件-插上旗帜 */
    virtual void on_flagging(FlagType);

    /* 城市：事件-产生生命元 */
    virtual void on_produce_live() {live_elements += 10; }

    /* 城市：事件-生命元被取走 */
    virtual int on_looted_elements() {int x = live_elements; live_elements = 0; return x; }

    /* 城市：事件-被到达 */
    virtual void on_reached(WarriorBase* w) {
        GameEngine::log(w->get_camp(), this, EVENT_MARCH, w->info().c_str(), info().c_str(), w->get_hp(), w->get_attack());
        warrior[int(w->get_camp())] = w;
    }
};

class HeadQuarter final : public City  {
private:
    /* 司令部：所在阵营 */
    Camp camp;
    /* 司令部：召唤的武士编号 */
    int index = 0;
    /* 司令部：当前应当召唤的武士 */
    int now_summon = 0;
    /* 司令部：当前已经攻占的敌方武士 */
    std::vector<WarriorBase*> taken;
public:
    HeadQuarter(Camp c, int id) : City(id), camp(c) {
        live_elements = GameEngine::get_config().life_elements;
    }
    ~HeadQuarter() {
        for (WarriorBase *w : taken)
            delete w;
    }

    /* 司令部：获取信息 */
    virtual std::string info() const override {return utils::format("%s headquarter", camp_name(camp).c_str()); }

    /* 司令部：是否为司令部 */
    virtual bool is_headquarter() const override {return true; }
    /* 司令部：获得阵营 */
    Camp get_camp() const {return camp; }
    /* 司令部：获得新的编号 */
    int new_index() {return ++index; }

    /* 司令部：是否被攻占 */
    bool be_taken() const {return taken.size() >= 2; }

    /* 司令部：事件-被攻占 */
    void on_taken() const {
        GameEngine::log(camp, this, EVENT_TAKEN, info().c_str());
    }

    /* 司令部：事件-被到达 */
    virtual void on_reached(WarriorBase* w) override {
        GameEngine::log(w->get_camp(), this, EVENT_ARRIVED, w->info().c_str(), info().c_str(), w->get_hp(), w->get_attack());
        taken.emplace_back(w);
        if (taken.size() == 2) on_taken();
    }

    /* 司令部：报告生命元数量 */
    void report() const {
        GameEngine::log(camp, this, EVENT_REPORT_ELEM, live_elements, info().c_str());
    }

    /* 司令部：滞留在司令部的武士报告 */
    void warrior_report() {
        for (WarriorBase *w : taken)
            w->report();
    }

    virtual void on_time_changing(utils::Timer time) override {
        switch (time.minute) {
            case 0: on_summon_warrior(); break;
            case 50: report(); break;
            case 55: warrior_report(); break;
        }
        City::on_time_changing(time);
    }

    /* 司令部：事件-尝试召唤武士*/
    void on_summon_warrior();

    /* 司令部：事件-奖励获胜武士 */
    void on_reward(WarriorBase* a) {
        int num = std::min(live_elements, 8);
        a->on_damaged(nullptr, {DAMAGE_HEAL, -num});
        live_elements -= num;
    }

    /* 司令部：不产生生命元*/
    void on_produce_live() override {}
    /* 司令部：事件-通过武士获取生命元 */
    void on_earn_elements(int w) {
        live_elements += w;
    }

};

class WeaponSword final : public WeaponBase {
private:

public:
    WeaponSword(WarriorBase* w) {damage = w->get_attack() / 5; }
    virtual int get_type() const override {return 0; }
    virtual std::string get_name() const override {return "sword"; }
    virtual void on_used() override {damage *= 0.8; }
    
};

class WeaponBomb final : public WeaponBase {
private:
public:
    WeaponBomb() {damage = 1; }
    virtual int get_type() const override {return 1; }
    virtual std::string get_name() const override {return "bomb"; }
    virtual std::string info() const override {return "bomb"; }
};

class WeaponArrow final : public WeaponBase {
private:
public:
    WeaponArrow() {damage = 3; }
    virtual int get_type() const override {return 2; }
    virtual std::string get_name() const override {return "arrow"; }
};

WarriorBase::WarriorBase(HeadQuarter *head) {
    this->id        = head->new_index();
    this->camp      = head->get_camp();
    this->now_at    = head;
    this->weapons   = new WeaponList();
    this->head_at   = head;
}
void WarriorBase::initialize() {
    this->attack    = GameEngine::get_config().initial_attack[get_type()];
    this->hp        = GameEngine::get_config().initial_hp[get_type()];
    this->head_at->on_earn_elements(-this->hp);
    this->on_born();
}

WarriorBase::~WarriorBase() {delete weapons; }

int WarriorBase::real_attack(bool active) const {
    WeaponBase *s = weapons->get(WEAPON_SWORD);
    return attack / (2 - active) + (s == nullptr ? 0 : s->get_damage());
}

WarriorBase* WarriorBase::get_rival() const {
    return now_at->get_warrior(rival(camp));
}

void WarriorBase::on_born() {
    GameEngine::log(camp, now_at, EVENT_BORN, info().c_str());
}

void WarriorBase::destroy() {
    City* c = this->now_at;
    now_at->remove(this->camp);
    delete this;
}

void WarriorBase::on_time_changing(utils::Timer time) {
    switch (time.minute) {
        case 30: on_loot_elements(); break;
        case 35: on_check_arrow(); break;
        case 38: on_check_bomb(); break;
        case 55: report(); break;
    }
}

City* WarriorBase::forward() const {
    return this->camp == CAMP_RED ? now_at->east() : now_at->west();
}

void WarriorBase::on_move_forward() {
    City *c = now_at;
    c->remove(camp);
    now_at = forward();
    now_at->on_reached(this);
}

bool WarriorBase::on_check_arrow() {
    if (weapons->get(WEAPON_ARROW) == nullptr)
        return false;
    City *d = forward();
    if (d == nullptr) return false;
    WarriorBase *r = d->get_warrior(rival(camp));
    if (r == nullptr) return false;
    on_use_arrow(r);
    return true;
}

void WarriorBase::on_use_arrow(WarriorBase* rival) {
    weapons->on_used(WEAPON_ARROW);
    bool dead = rival->on_damaged(this, {DAMAGE_ARROW, GameEngine::get_config().arrow_attack});

    if (dead) {
        GameEngine::log(camp, now_at, EVENT_ARROW_KILL, info().c_str(), rival->info().c_str());
    } else {
        GameEngine::log(camp, now_at, EVENT_ARROW, info().c_str());
    }
}


bool WarriorBase::on_check_bomb() {
    if (weapons->get(WEAPON_BOMB) == nullptr)
        return false; 
    City *c = now_at;
    WarriorBase *r = get_rival();
    if (r == nullptr) return false;
    if (hp == 0 || r->hp == 0) return false;
    bool active = c->which_active() == camp;
    if (active) {
        if (real_attack(true) >= r->hp) return false;
        if (r->real_attack(false) < hp) return false;
    } else {
        if (r->real_attack(true) < hp) return false;
    }
    on_use_bomb(r);
    return true;
}

void WarriorBase::on_use_bomb(WarriorBase *r) {
    GameEngine::log(camp, now_at, EVENT_BOMB, info().c_str(), r->info().c_str());
    r->destroy();
    this->destroy();
}

bool WarriorBase::on_damaged(WarriorBase *r, Damage d) {
    hp = std::max(0, hp - d.dmg);
    return hp == 0;
}

void WarriorBase::on_attack(WarriorBase *r) {
    GameEngine::log(camp, now_at, EVENT_ATTACK, info().c_str(), r->info().c_str(), now_at->info().c_str(), hp, attack);

    int atk = real_attack(true);
    r->on_damaged(this, {DAMAGE_SWORD, atk});
    weapons->on_used(WEAPON_SWORD);
}

void WarriorBase::on_fight_back(WarriorBase *r) {
    GameEngine::log(camp, now_at, EVENT_FIGHT_BACK, info().c_str(), r->info().c_str(), now_at->info().c_str());

    int atk = real_attack(false);
    r->on_damaged(this, {DAMAGE_SWORD, atk});
    weapons->on_used(WEAPON_SWORD);
}

void WarriorBase::on_win(WarriorBase *r, bool active) {
}

void WarriorBase::on_dead(WarriorBase *r) {
    GameEngine::log(camp, now_at, EVENT_DEAD, info().c_str(), now_at->info().c_str());

    this->destroy();
}

void WarriorBase::on_loot_elements() {
    City *c = now_at;
    WarriorBase *r = get_rival();
    if (r != nullptr && r->get_hp() > 0) return;
    int w = c->on_looted_elements();
    if (w) {
        head_at->on_earn_elements(w);
        GameEngine::log(camp, now_at, EVENT_EARN, info().c_str(), w);
    }
}

void WarriorBase::report() const {
    GameEngine::log(camp, now_at, EVENT_REPORT_WEAP, info().c_str(), weapons->report().c_str());
}

void City::clear() {
    for (Camp c : CAMPS)
        if (get_warrior(c) != nullptr && get_warrior(c)->get_hp() == 0)
            get_warrior(c)->destroy();
}

void City::on_time_changing(utils::Timer time) {
    switch (time.minute) {
        case 20: on_produce_live(); break;
    }
}

BattleType City::battle(WarriorBase *active, WarriorBase *passive) {
    /* 战斗前两人都死了则无事发生 */
    if (active->get_hp() == 0 && passive->get_hp() == 0)
        return BATTLE_NONE;
    /* 判断战斗开始前，是否有人阵亡（此时不算战死） */
    if (active->get_hp() == 0) {
        passive->on_win(active, false);
        return BattleType(passive->get_camp());
    }
    if (passive->get_hp() == 0) {
        active->on_win(passive, true);
        return BattleType(active->get_camp());
    }
    /* 先手开始攻击 */
    active->on_attack(passive);
    if (passive->get_hp() == 0) {
        active->on_win(passive, true);
        passive->on_dead(active);
        return BattleType(active->get_camp());
    }
    /* 后手开始反击 */
    passive->on_fight_back(active);
    if (active->get_hp() == 0) {
        passive->on_win(active, false);
        active->on_dead(passive);
        return BattleType(passive->get_camp());
    }
    /* 平局 */
    active->on_draw(passive, true);
    passive->on_draw(active, false);
    return BATTLE_DRAW;
}

Camp City::which_active() const {
    if (flag != FLAG_NONE)
        return Camp(flag);
    else
        return id & 1 ? CAMP_RED : CAMP_BLUE;
}

void City::on_battle() {
    /* 不足两人则不开战 */
    for (Camp c : CAMPS)
        if (get_warrior(c) == nullptr) {
            last_battle = BATTLE_NONE;
            return;
        }
    /* 判断主动方并开战 */
    Camp act = which_active();
    BattleType result = battle(get_warrior(act), get_warrior(rival(act)));

    /* 记录上一回合战斗情况 */
    last_battle = result;
    if (result == BATTLE_NONE) return;

    /* 同阵营连续胜利两场进行插旗 */
    if (result != BATTLE_DRAW && result == history_battle && result != BattleType(flag)) {
        on_flagging(FlagType(result));
    }
    history_battle = result;
}

void City::on_flagging(FlagType f) {
    flag = f;
    Camp c = Camp(f);
    GameEngine::log(c, this, EVENT_FLAG, camp_name(c).c_str(), info().c_str());
}

/* 武器：生成新的武器 */
WeaponBase* new_weapon(int id, WarriorBase *w) {
    WeaponBase *weap = nullptr;
    switch (id) {
        case 0: weap = new WeaponSword(w); break;
        case 1: weap = new WeaponBomb(); break;
        case 2: weap = new WeaponArrow(); break;
    }
    if (weap->is_damaged()) {
        delete weap;
        return nullptr;
    } else if (weap != nullptr) {
        return weap;
    }
    assert(("Unknown Weapon ID", false));
    return nullptr;
}

class WarriorDragon final : public WarriorBase {
private:
    double morale;
public:
    virtual int get_type() const override {return 0; }
    virtual std::string get_name() const override {return "dragon"; }

    WarriorDragon(HeadQuarter *hq) : WarriorBase(hq) {
        initialize();
        weapons->insert(new_weapon(id % 3, this));
    }
    virtual void on_born() override {
        morale = (double) head_at->get_elem() / GameEngine::get_config().initial_hp[get_type()];
        GameEngine::log(camp, now_at, EVENT_BORN_DRAGON, info().c_str(), morale);
    }
    void on_cheer() {
        GameEngine::log(camp, now_at, EVENT_CHEER, info().c_str(), now_at->info().c_str());
    }
    virtual void on_win(WarriorBase *r, bool active) override {
        morale += 0.2;
        if (active && morale >= 0.8)
            on_cheer();
        WarriorBase::on_win(r, active);
    }
    virtual void on_draw(WarriorBase *r, bool active) {
        morale -= 0.2;
        if (active && morale >= 0.8)
            on_cheer();
        WarriorBase::on_draw(r, active);
    }
};

class WarriorNinja final : public WarriorBase {
private:
public:
    virtual int get_type() const override {return 1; }
    virtual std::string get_name() const override {return "ninja"; }

    WarriorNinja(HeadQuarter *hq) : WarriorBase(hq) {
        initialize();
        weapons->insert(new_weapon(id % 3, this));
        weapons->insert(new_weapon((id + 1) % 3, this));
    }
    virtual void on_fight_back(WarriorBase *r) override {}
    virtual int real_attack(bool active) const override {
        return active ? WarriorBase::real_attack(active) : 0;
    }
};

class WarriorIceman final : public WarriorBase {
private:
    int walked;
public:
    virtual int get_type() const override {return 2; }
    virtual std::string get_name() const override {return "iceman"; }

    WarriorIceman(HeadQuarter *hq) : WarriorBase(hq) {
        initialize();
        walked = 0;
        weapons->insert(new_weapon(id % 3, this));
    }
    virtual void on_move_forward() override {
        walked ^= 1;
        if (walked == 0) {
            on_damaged(nullptr, {DAMAGE_ICEMAN, std::min(hp - 1, 9)});
            attack += 20;
        }
        WarriorBase::on_move_forward();
    }
};

class WarriorLion final : public WarriorBase {
private:
    int loyalty, history_hp;
public:
    virtual int get_type() const override {return 3; }
    virtual std::string get_name() const override {return "lion"; }

    WarriorLion(HeadQuarter *hq) : WarriorBase(hq) {
        initialize();
    }

    virtual void on_born() override {
        loyalty = head_at->get_elem();
        GameEngine::log(camp, now_at, EVENT_BORN_LION, info().c_str(), loyalty);
        history_hp = hp;
    }
    virtual void on_win(WarriorBase *r, bool active) override {
        history_hp = hp;
        WarriorBase::on_win(r, active);
    }
    virtual void on_draw(WarriorBase *r, bool active) override {
        loyalty -= GameEngine::get_config().loyalty_dec;
        history_hp = hp;
        WarriorBase::on_draw(r, active);
    }
    virtual bool on_damaged(WarriorBase *r, Damage d) override {
        bool ret = WarriorBase::on_damaged(r, d);
        /* 若非战斗时的普通攻击，则记录下改变后血量 */
        if (d.type != DAMAGE_SWORD)
            history_hp = hp;
        return ret;
    }
    virtual void on_dead(WarriorBase *r) override {
        /* lion 战死后血量被窃取 */
        r->on_damaged(this, {DAMAGE_HEAL, -history_hp});
        WarriorBase::on_dead(r);
    }
    void on_escape() {
        GameEngine::log(camp, now_at, EVENT_ESCAPE, info().c_str());
        destroy();
    }
    virtual void on_time_changing(utils::Timer time) override {
        if (time.minute == 5 && loyalty <= 0)
            on_escape();
        else
            WarriorBase::on_time_changing(time);
    }
};

class WarriorWolf final : public WarriorBase {
private:
public:
    virtual int get_type() const override {return 4; }
    virtual std::string get_name() const override {return "wolf"; }

    WarriorWolf(HeadQuarter *hq) : WarriorBase(hq) {
        initialize();
    }

    virtual void on_win(WarriorBase *r, bool active) override {
        weapons->on_merge(r->get_weapons());
        WarriorBase::on_win(r, active);
    }
};

WarriorBase* new_warrior(int id, HeadQuarter *hq) {
    WarriorBase *ret = nullptr;
    switch (id) {
        case 0: return new WarriorDragon(hq);
        case 1: return new WarriorNinja(hq);
        case 2: return new WarriorIceman(hq);
        case 3: return new WarriorLion(hq);
        case 4: return new WarriorWolf(hq);
    }
    assert(("Unknown Warrior ID", false));
    return nullptr;
}

void HeadQuarter::on_summon_warrior() {
    const auto &order = GameEngine::get_config().summon_order[camp];
    if (live_elements < GameEngine::get_config().initial_hp[order[now_summon]])
        return;
    warrior[camp] = new_warrior(order[now_summon], this);
    now_summon = (now_summon + 1) % order.size();
}

GameEngine::~GameEngine() {
    for (City *c : cities)
        delete c;
}

template<typename ...Args>
void GameEngine::log(const Camp camp_id, const City* city, const EType event, Args && ...params) {
    std::string s = utils::format(event.fmt, std::forward<Args>(params)...);
    utils::Event e(time(), event.id, camp_id, city->get_id(), s);
    log(e);
}

void GameEngine::initialize() {
    events.clear();
    cities.clear();

    int N = config.city_num;

    cities.resize(N + 2);
    cities[0] = hq[CAMP_RED] = new HeadQuarter(CAMP_RED, 0);
    cities[N + 1] = hq[CAMP_BLUE] = new HeadQuarter(CAMP_BLUE, N + 1);
    for (int i = 1; i <= N; ++i)
        cities[i] = new City(i);
    
    for (int i = 0; i <= N + 1; ++i)
        cities[i]->set_neighbors(i == 0 ? nullptr : cities[i - 1],
                                 i == N + 1 ? nullptr : cities[i + 1]);
}

void GameEngine::start() {

    GameEngine* game = instance;

    game->initialize();

    utils::Timer& time = game->now_time;

    bool stopped = false;

    for (time.hour = 0; !stopped; ++time.hour) {
        for (time.minute = 0; time.minute < 60; ++time.minute) {
            /* 超时退出 */
            if (time.time() > game->config.stop_time)
                { stopped = true; break; }
            /* 被占领退出 */
            if (game->hq[CAMP_RED]->be_taken() || game->hq[CAMP_BLUE]->be_taken())
                { stopped = true; break; }

            /* 无序事件过程（通过检测时间完成） */
            for (City *city : game->cities) {
                city->on_time_changing(time);
                for (Camp camp : CAMPS) {
                    WarriorBase *w = city->get_warrior(camp);
                    if (w == nullptr) continue;
                    w->on_time_changing(time);
                }
            }

            /* 有序事件过程 */
            switch (time.minute) {
                /* 10 时刻：武士前进 */
                case 10:
                    /* 红方行走 */
                    std::for_each(game->cities.rbegin(), game->cities.rend(), [] (City *c) {
                        if (c->get_warrior(CAMP_RED) != nullptr)
                            c->get_warrior(CAMP_RED)->on_move_forward();
                    });
                    /* 蓝方行走 */
                    std::for_each(game->cities.begin(), game->cities.end(), [] (City *c) {
                        if (c->get_warrior(CAMP_BLUE) != nullptr)
                            c->get_warrior(CAMP_BLUE)->on_move_forward();
                    });
                    break;
                case 40:
                    /* 发生战斗 */
                    for (City *c : game->cities) 
                        c->on_battle();
                    /* 红方发送奖励 */
                    std::for_each(game->cities.rbegin(), game->cities.rend(), [] (City *c) {
                        WarriorBase *w = c->get_winner();
                        if (w != nullptr && w->get_camp() == CAMP_RED)
                            w->get_head()->on_reward(w);
                    });
                    /* 蓝方发送奖励 */
                    std::for_each(game->cities.begin(), game->cities.end(), [] (City *c) {
                        WarriorBase *w = c->get_winner();
                        if (w != nullptr && w->get_camp() == CAMP_BLUE)
                            w->get_head()->on_reward(w);
                    });
                    /* 武士夺取城市生命元 */
                    for (City *c : game->cities) {
                        WarriorBase *w = c->get_winner();
                        if (w != nullptr) w->on_loot_elements();
                    }
                    /* 清理尸体 */
                    for (City *c : game->cities)
                        c->clear();
                    break;
            }
        }
    }
}

};

int main(int argc, char *argv[]) {
    std::ios::sync_with_stdio(0), std::cin.tie(0);
    int test_cases;
    std::cin >> test_cases;
    for (int id = 1; id <= test_cases; ++id) {
        std::cout << "Case " << id << ":\n";

        WoW::GameConfig config;

        std::cin >> config.life_elements >> config.city_num >> config.arrow_attack >> config.loyalty_dec >> config.stop_time;
        for (int i = 0; i < 5; ++i) std::cin >> config.initial_hp[i];
        for (int i = 0; i < 5; ++i) std::cin >> config.initial_attack[i];

        config.summon_order[WoW::CAMP_RED] = {2, 3, 4, 1, 0};
        config.summon_order[WoW::CAMP_BLUE] = {3, 0, 1, 2, 4};

        WoW::GameEngine::load_settings(config);
        WoW::GameEngine::start();
        WoW::GameEngine::output();
        WoW::GameEngine::stop();
    }
    return 0;
}