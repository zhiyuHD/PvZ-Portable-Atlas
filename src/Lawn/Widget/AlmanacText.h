#ifndef __ALMANACTEXT_H__
#define __ALMANACTEXT_H__

// UTF-8 text used only by the Almanac. Global game strings stay independent.
struct AlmanacEntryText
{
    const char* name;
    const char* description;
};

inline constexpr AlmanacEntryText kAlmanacPlants[] = {
    {"豌豆射手", R"ZH(豌豆射手是你的第一道防线，会向来袭的僵尸发射豌豆。

{KEYWORD}伤害：{STAT}普通{NORMAL}

{FLAVOR}一株植物怎么能这么快地长出、射出那么多豌豆？豌豆射手说：“靠的是勤奋、毅力，以及一顿营养均衡的早餐——阳光配高纤维二氧化碳。”)ZH"},
    {"向日葵", R"ZH(向日葵能提供额外的阳光，是必不可少的帮手。尽量多种几株吧！

{KEYWORD}阳光产量：{STAT}普通{NORMAL}

{FLAVOR}向日葵总忍不住随着节拍摇摆。什么节拍？当然是大地本身充满生机的爵士节奏。那股律动，只有向日葵听得见。)ZH"},
    {"樱桃炸弹", R"ZH(樱桃炸弹能炸掉一片区域内的僵尸。引线很短，请种在僵尸附近。

{KEYWORD}伤害：{STAT}极高{NORMAL}
{KEYWORD}范围：{STAT}中等区域{NORMAL}
{KEYWORD}用法：{STAT}一次性，立即生效{NORMAL}

{FLAVOR}“我要爆炸！”樱桃一号说。“不，咱们应该引爆！”他的兄弟樱桃二号说。经过一番激烈讨论，他们决定来个“爆引炸”。)ZH"},
    {"坚果", R"ZH(坚果有坚硬的外壳，可以保护其他植物。

{KEYWORD}耐久：{STAT}高{NORMAL}

{FLAVOR}“大家都想知道，被僵尸不停啃咬是什么感觉。”坚果说，“他们不知道，我的感官不太灵敏，只能感觉到一点麻麻的，就像舒服的背部按摩。”)ZH"},
    {"土豆地雷", R"ZH(土豆地雷威力很大，但需要时间准备。请提前种在僵尸前方，准备好后触碰即爆。

{KEYWORD}伤害：{STAT}极高{NORMAL}
{KEYWORD}范围：{STAT}小范围{NORMAL}
{KEYWORD}用法：{STAT}一次性，延迟就绪{NORMAL}

{FLAVOR}有人说土豆地雷懒，总把事情拖到最后一刻。土豆地雷一言不发。他正忙着琢磨自己的投资策略呢。)ZH"},
    {"寒冰射手", R"ZH(寒冰射手发射冰冻豌豆，造成伤害并减缓敌人的速度。

{KEYWORD}伤害：{STAT}普通，附带减速{NORMAL}

{FLAVOR}大家总说寒冰射手“酷得很”，劝他“冷静点”，叫他“保持冰爽”。寒冰射手只翻了个白眼。这些冷笑话，他早听遍了。)ZH"},
    {"大嘴花", R"ZH(大嘴花能一口吞下僵尸，但咀嚼时很容易受到攻击。

{KEYWORD}伤害：{STAT}极高{NORMAL}
{KEYWORD}范围：{STAT}很近{NORMAL}
{KEYWORD}特点：{STAT}吞噬后需要很长时间消化{NORMAL}

{FLAVOR}大嘴花差点就接到《异形奇花》的特技演员工作，可他的经纪人要的预付款太高，事情黄了。大嘴花倒不记恨谁。他说，这就是这一行的规矩。)ZH"},
    {"双发射手", R"ZH(双发射手每次发射两颗豌豆。

{KEYWORD}伤害：{STAT}每颗普通{NORMAL}
{KEYWORD}射速：{STAT}两倍{NORMAL}

{FLAVOR}双发射手很凶。他在街头长大，不管对方是植物还是僵尸，都别想给他脸色看。他用豌豆让别人保持距离。不过，他心里其实很渴望爱。)ZH"},
    {"小喷菇", R"ZH(小喷菇价格低廉，但只能攻击近处的敌人。

{KEYWORD}伤害：{STAT}普通{NORMAL}
{KEYWORD}范围：{STAT}近距离{NORMAL}
{NOCTURNAL}白天睡觉{NORMAL}

{FLAVOR}“我最近才知道僵尸真的存在。”小喷菇说，“和许多真菌一样，我一直以为那只是童话或电影里的怪物。这次经历可真让我开了眼。”)ZH"},
    {"阳光菇", R"ZH(阳光菇起初产生小阳光，长大后产生普通阳光。

{KEYWORD}阳光产量：{STAT}起初少，随后普通{NORMAL}
{NOCTURNAL}白天睡觉{NORMAL}

{FLAVOR}阳光菇讨厌阳光。讨厌到阳光只要在体内积起来，他就要赶紧吐出去。他一点也容不下它。在他看来，阳光实在太俗气了。)ZH"},
    {"大喷菇", R"ZH(大喷菇喷出能穿透铁栅门的烟雾。

{KEYWORD}伤害：{STAT}普通，穿透铁栅门{NORMAL}
{KEYWORD}范围：{STAT}烟雾中的所有僵尸{NORMAL}
{NOCTURNAL}白天睡觉{NORMAL}

{FLAVOR}“以前我给一家面包店生产酵母孢子，那工作毫无前途。”大喷菇说，“后来，多亏小喷菇告诉我，有份轰击僵尸的好差事。现在我总算觉得自己能改变点什么了。”)ZH"},
    {"墓碑吞噬者", R"ZH(将墓碑吞噬者种在墓碑上，就能移除墓碑。

{KEYWORD}用法：{STAT}一次性，种在墓碑上{NORMAL}
{KEYWORD}特点：{STAT}移除墓碑{NORMAL}

{FLAVOR}虽然长相吓人，墓碑吞噬者还是想让大家知道：他喜欢小猫，休息时还会去当地的僵尸康复中心做志愿者。“这是应该做的。”他说。)ZH"},
    {"魅惑菇", R"ZH(僵尸吃下魅惑菇后，会转身为你作战。

{KEYWORD}用法：{STAT}一次性，被吃后生效{NORMAL}
{KEYWORD}特点：{STAT}使僵尸为你作战{NORMAL}
{NOCTURNAL}白天睡觉{NORMAL}

{FLAVOR}“僵尸是我们的朋友。”魅惑菇坚持说，“它们被严重误解了，其实在生态系统中扮演着重要角色。我们能够，也应该，多做些事，让它们接受我们的想法。”)ZH"},
    {"胆小菇", R"ZH(胆小菇能攻击远处的敌人，但敌人靠近时会躲起来。

{KEYWORD}伤害：{STAT}普通{NORMAL}
{KEYWORD}特点：{STAT}敌人靠近时停止攻击{NORMAL}
{NOCTURNAL}白天睡觉{NORMAL}

{FLAVOR}“谁在那儿？”胆小菇轻声问，声音小得几乎听不见。“走开。我不想见任何人。除非是马戏团的人。”)ZH"},
    {"寒冰菇", R"ZH(寒冰菇能暂时冻住屏幕上的所有僵尸。

{KEYWORD}伤害：{STAT}很低，附带冻结{NORMAL}
{KEYWORD}范围：{STAT}全屏{NORMAL}
{KEYWORD}用法：{STAT}一次性，立即生效{NORMAL}
{NOCTURNAL}白天睡觉{NORMAL}

{FLAVOR}寒冰菇皱着眉，并不是因为不开心，也不是不赞同谁。他小时候受过伤，面部神经因此瘫痪了。)ZH"},
    {"毁灭菇", R"ZH(毁灭菇能摧毁大片区域内的敌人，并留下无法种植的弹坑。

{KEYWORD}伤害：{STAT}极高{NORMAL}
{KEYWORD}范围：{STAT}大范围{NORMAL}
{KEYWORD}用法：{STAT}一次性，立即生效{NORMAL}
{KEYWORD}特点：{STAT}留下弹坑{NORMAL}
{NOCTURNAL}白天睡觉{NORMAL}

{FLAVOR}“幸好我是站在你这边的。”毁灭菇说，“你珍惜的一切，我都能摧毁。还不费什么劲。”)ZH"},
    {"荷叶", R"ZH(荷叶能让非水生植物种在它上面。

{KEYWORD}特点：{STAT}承载非水生植物{NORMAL}
{AQUATIC}必须种在水中{NORMAL}

{FLAVOR}荷叶从不抱怨，也从不追问发生了什么。在它身上放一株植物，它也不会吭声。它有没有惊人的见解，或者不可告人的秘密？没人知道。荷叶把一切都藏在心里。)ZH"},
    {"窝瓜", R"ZH(窝瓜会压扁第一个靠近它的僵尸。

{KEYWORD}伤害：{STAT}极高{NORMAL}
{KEYWORD}范围：{STAT}近距离，压中位置的所有僵尸{NORMAL}
{KEYWORD}用法：{STAT}一次性{NORMAL}

{FLAVOR}“我准备好了！”窝瓜大喊，“来吧！让我上！没人比我更行！我就是你要找的！快点！还等什么？我需要这个机会！”)ZH"},
    {"三线射手", R"ZH(三线射手同时向三行发射豌豆。

{KEYWORD}伤害：{STAT}每颗普通{NORMAL}
{KEYWORD}范围：{STAT}三行{NORMAL}

{FLAVOR}三线射手喜欢阅读、西洋双陆棋，以及在公园里长时间一动不动。他喜欢看演出，尤其是现代爵士乐。“我只是在寻找那个特别的人。”他说。三线射手最喜欢的数字是五。)ZH"},
    {"缠绕海草", R"ZH(缠绕海草是一种水生植物，会把第一个靠近的僵尸拖入水中。

{KEYWORD}伤害：{STAT}极高{NORMAL}
{KEYWORD}用法：{STAT}一次性，接触生效{NORMAL}
{AQUATIC}必须种在水中{NORMAL}

{FLAVOR}“我完全隐形了。”缠绕海草心想，“我藏在水面下面，谁也看不见我。”朋友们说，他们看得清清楚楚。但他就是不改。)ZH"},
    {"火爆辣椒", R"ZH(火爆辣椒能摧毁一整行的僵尸。

{KEYWORD}伤害：{STAT}极高{NORMAL}
{KEYWORD}范围：{STAT}一整行{NORMAL}
{KEYWORD}用法：{STAT}一次性，立即生效{NORMAL}

{FLAVOR}“嗯嗯嗯啊啊啊啊！！！”火爆辣椒说。他还不会爆炸，现在还不会。但快了，真的快了。那一刻近在眼前，他知道，他能感觉到，他的一生都在为这一刻铺路。)ZH"},
    {"地刺", R"ZH(地刺能扎破轮胎，并伤害踩上来的僵尸。

{KEYWORD}伤害：{STAT}普通{NORMAL}
{KEYWORD}范围：{STAT}经过的所有僵尸{NORMAL}
{KEYWORD}特点：{STAT}不会被啃食{NORMAL}

{FLAVOR}地刺痴迷冰球。他买了包厢座位的赛季票，紧盯着喜欢的球员，在办公室的冰球竞猜里总是大获全胜。只有一个问题：他怕冰球怕得要命。)ZH"},
    {"火炬树桩", R"ZH(火炬树桩把经过的豌豆变成火球，伤害翻倍。

{KEYWORD}特点：{STAT}经过的豌豆伤害翻倍，火球命中时伤害附近僵尸{NORMAL}

{FLAVOR}大家都喜欢并尊敬火炬树桩，因为他正直、友谊可靠，还能大大提高豌豆的伤害。不过，火炬树桩有个秘密：他不识字。)ZH"},
    {"高坚果", R"ZH(高坚果是一堵坚实的屏障，无法被撑杆或跳跃越过。

{KEYWORD}耐久：{STAT}很高{NORMAL}
{KEYWORD}特点：{STAT}阻止跨越和跳跃{NORMAL}

{FLAVOR}有人猜坚果和高坚果是不是互相较劲。高坚果用浑厚的男低音笑起来：“我们之间能有什么矛盾？我们是兄弟。要是你知道坚果为我做过什么……”他的声音渐渐低下去，露出了会心的微笑。)ZH"},
    {"海蘑菇", R"ZH(海蘑菇是一种水生植物，会发射近距离孢子。

{KEYWORD}伤害：{STAT}普通{NORMAL}
{KEYWORD}范围：{STAT}近距离{NORMAL}
{AQUATIC}必须种在水中{NORMAL}
{NOCTURNAL}白天睡觉{NORMAL}

{FLAVOR}海蘑菇从没见过大海。名字里有“海”，也听过不少关于海的故事，可他就是一直抽不出时间。不过，总有一天，他会去的。)ZH"},
    {"路灯花", R"ZH(路灯花照亮周围，让你看清迷雾中的情况。

{KEYWORD}范围：{STAT}所在行{NORMAL}
{KEYWORD}特点：{STAT}照亮迷雾{NORMAL}

{FLAVOR}路灯花就是不讲科学。别的植物吸收光、释放氧气；他吸收黑暗、释放光。至于怎么办到的，他总是闪烁其词：“我可没说这是‘巫术’，也不会用‘黑暗力量’这种词。我只是……我想我已经说得够多了。”)ZH"},
    {"仙人掌", R"ZH(仙人掌发射尖刺，可以攻击地面和空中的目标。

{KEYWORD}伤害：{STAT}普通{NORMAL}
{KEYWORD}范围：{STAT}地面和空中{NORMAL}

{FLAVOR}仙人掌确实浑身带刺，但尖刺之下，是一颗柔软、充满爱与善意的心。她只想拥抱别人，也被别人拥抱。多数人受不了，可她不介意。她最近在和一只犰狳约会，看起来还挺合得来。)ZH"},
    {"三叶草", R"ZH(三叶草能吹走所有气球僵尸和迷雾。

{KEYWORD}用法：{STAT}一次性，立即生效{NORMAL}
{KEYWORD}特点：{STAT}吹走所有气球僵尸{NORMAL}

{FLAVOR}五岁生日那年，三叶草收到一个漂亮的新蛋糕。他许了愿，使劲吹啊吹，却只吹灭了六成蜡烛。他没有放弃，从那以后，他一直把这次失败当作激励自己更努力的动力。)ZH"},
    {"裂荚射手", R"ZH(裂荚射手能向前后两个方向发射豌豆。

{KEYWORD}伤害：{STAT}普通{NORMAL}
{KEYWORD}范围：{STAT}前后两侧{NORMAL}
{KEYWORD}射速：{STAT}前方一倍，后方两倍{NORMAL}

{FLAVOR}“没错，我是双子座。”裂荚射手说，“我知道，你肯定一点也不意外。但有两个脑袋——准确地说，是一个脑袋，后面长了个很像脑袋的大包——在我这行还真挺有用的。”)ZH"},
    {"杨桃", R"ZH(杨桃向五个方向发射星星。

{KEYWORD}伤害：{STAT}普通{NORMAL}
{KEYWORD}范围：{STAT}五个方向{NORMAL}

{FLAVOR}“哎，真服了。”杨桃说，“前几天去看牙医，他说我有四处蛀牙。你数数，我只有一颗牙！一颗牙，四个洞？这是怎么回事？”)ZH"},
    {"南瓜头", R"ZH(南瓜头能保护壳里的植物。

{KEYWORD}耐久：{STAT}高{NORMAL}
{KEYWORD}特点：{STAT}可以套在其他植物外面{NORMAL}

{FLAVOR}南瓜头最近没收到表兄伦菲尔德的消息。听说他成了大明星，好像是什么……怎么说来着……体育英雄？《幻幻球》大师？南瓜头不太懂。他只是做好自己的工作。)ZH"},
    {"磁力菇", R"ZH(磁力菇能吸走僵尸的头盔和其他金属物品。

{KEYWORD}范围：{STAT}附近僵尸{NORMAL}
{KEYWORD}特点：{STAT}移除金属物品{NORMAL}
{NOCTURNAL}白天睡觉{NORMAL}

{FLAVOR}磁力是一股强大的力量。非常强大。有时候，磁力菇自己都有点怕。他不确定自己是否承担得起这样的责任。)ZH"},
    {"卷心菜投手", R"ZH(卷心菜投手向敌人投掷卷心菜。

{KEYWORD}伤害：{STAT}普通{NORMAL}
{KEYWORD}范围：{STAT}抛物线投射{NORMAL}

{FLAVOR}卷心菜投手不介意把卷心菜扔向僵尸。毕竟，这就是他的本职工作，而且他干得不错。他只是搞不懂，僵尸一开始到底是怎么爬上屋顶的。)ZH"},
    {"花盆", R"ZH(花盆让你能够在屋顶上种植。

{KEYWORD}特点：{STAT}让植物种在屋顶上{NORMAL}

{FLAVOR}“我是用来种植物的盆，但我自己也是植物。你的脑子是不是已经炸了？”)ZH"},
    {"玉米投手", R"ZH(玉米投手向僵尸投掷玉米粒和黄油。

{KEYWORD}伤害：{STAT}玉米粒低，黄油普通{NORMAL}
{KEYWORD}范围：{STAT}抛物线投射{NORMAL}
{KEYWORD}特点：{STAT}黄油能定住僵尸{NORMAL}

{FLAVOR}玉米投手是投手三兄弟中的老大。三兄弟里，只有他总记得另外两个的生日。他还会为这事念叨他们几句。)ZH"},
    {"咖啡豆", R"ZH(咖啡豆可以唤醒睡着的蘑菇。

{KEYWORD}用法：{STAT}一次性，立即生效{NORMAL}
{KEYWORD}特点：{STAT}种在蘑菇上，唤醒蘑菇{NORMAL}

{FLAVOR}“嘿，伙计们，嘿！”咖啡豆说，“嘿！怎么啦？那是谁？嘿！看见那东西没？什么东西？哇！狮子！”没错，咖啡豆就是这么亢奋。)ZH"},
    {"大蒜", R"ZH(大蒜能让僵尸转移到其他行。

{KEYWORD}用法：{STAT}被啃咬时生效{NORMAL}
{KEYWORD}特点：{STAT}迫使僵尸换行{NORMAL}

{FLAVOR}引导换行不只是大蒜的职业，更是他的热爱。他拥有布鲁塞尔大学的转向学博士学位，能把行向量和排斥阵列讲上一整天。在家里，他也爱把东西推到另一条路上。真不知道他妻子是怎么受得了的。)ZH"},
    {"叶子保护伞", R"ZH(叶子保护伞能保护附近植物，抵挡飞贼和投石车的攻击。

{KEYWORD}特点：{STAT}保护周围植物免受飞贼偷取和投石车攻击{NORMAL}

{FLAVOR}“弹！”叶子保护伞说，“喜欢吗？我还能来一次。弹！哇！这就是我弹起来保护周围东西的样子。对，就这样。就是这样。信我。”)ZH"},
    {"金盏花", R"ZH(金盏花能产出银币和金币。

{KEYWORD}特点：{STAT}产出硬币{NORMAL}

{FLAVOR}金盏花花很多时间决定该吐银币还是金币。她仔细思考，权衡各个角度，认真做研究，还及时阅读最新刊物。赢家就是这样保持领先的。)ZH"},
    {"西瓜投手", R"ZH(西瓜投手能对成群僵尸造成大量伤害。

{KEYWORD}伤害：{STAT}高{NORMAL}
{KEYWORD}范围：{STAT}抛物线投射{NORMAL}
{KEYWORD}特点：{STAT}命中时伤害附近敌人{NORMAL}

{FLAVOR}西瓜投手从不假装谦虚。“按每单位阳光换来的伤害算，草坪上就数我最能打。”他说，“我没吹牛。你算算就知道。”)ZH"},
    {"机枪射手", R"ZH(机枪射手每次发射四颗豌豆。

{KEYWORD}伤害：{STAT}每颗普通{NORMAL}
{KEYWORD}射速：{STAT}四倍{NORMAL}
{KEYWORD}必须种在双发射手上{NORMAL}

{FLAVOR}机枪射手宣布要参军时，父母很担心。“可是，宝贝，那太危险了！”他们异口同声地说。机枪射手毫不动摇，眼神中闪着钢铁般的坚定：“生活本来就很危险。”)ZH"},
    {"双子向日葵", R"ZH(双子向日葵产生的阳光是普通向日葵的两倍。

{KEYWORD}阳光产量：{STAT}双倍{NORMAL}
{KEYWORD}必须种在向日葵上{NORMAL}

{FLAVOR}双子向日葵诞生于一个疯狂的禁忌科学之夜。雷声轰鸣，诡异的光闪烁，连狂风都像在愤怒地嘶吼、拒绝这一切。但都无济于事。双子向日葵活了，真的活了！)ZH"},
    {"忧郁菇", R"ZH(忧郁菇向周围释放浓烈的烟雾。

{KEYWORD}必须种在大喷菇上{NORMAL}

{FLAVOR}“我一直喜欢喷浓烟。”忧郁菇说，“我知道，很多人不喜欢。他们说这样没礼貌，或者味道太难闻。我只能问一句：难道你更想让僵尸吃掉脑子？”)ZH"},
    {"香蒲", R"ZH(香蒲能攻击任何一行，也能击落气球僵尸。

{KEYWORD}必须种在荷叶上{NORMAL}

{FLAVOR}“汪！”香蒲说，“汪汪汪！困惑了吗？就因为我的英文名字里有‘猫’，长得也像猫，你就觉得我该‘喵喵’叫？这里可不是这么回事。我拒绝被贴标签。”)ZH"},
    {"冰瓜", R"ZH(冰瓜能对成群僵尸造成大量伤害并减速。

{KEYWORD}伤害：{STAT}很高{NORMAL}
{KEYWORD}范围：{STAT}抛物线投射{NORMAL}
{KEYWORD}射速：{STAT}二分之一{NORMAL}
{KEYWORD}特点：{STAT}命中时伤害并冰冻减速附近敌人{NORMAL}
{KEYWORD}必须种在西瓜投手上{NORMAL}

{FLAVOR}冰瓜努力让自己镇定下来。他听见僵尸正在逼近。他能撑住吗？还有谁能撑住吗？)ZH"},
    {"吸金磁", R"ZH(吸金磁能替你收集硬币和钻石。

{KEYWORD}必须种在磁力菇上{NORMAL}

{FLAVOR}“我怎么会沦落到这里？”吸金磁问，“我本来前途无量，有独立的转角办公室、齐全的福利，还有股票期权。我都快当上中西部业务副总裁了。现在却在这片草坪上，随时可能被活活吃掉。哦！一枚硬币！”)ZH"},
    {"地刺王", R"ZH(地刺王能扎破多个轮胎，并伤害经过的僵尸。

{KEYWORD}必须种在地刺上{NORMAL}

{FLAVOR}地刺王刚从欧洲旅行回来。他玩得很开心，认识了许多好人，眼界也开阔了。他从不知道博物馆能造得那么大，里面还能挂那么多画。这可让他大吃一惊。)ZH"},
    {"玉米加农炮", R"ZH(点击玉米加农炮，即可发射致命的玉米炮弹。

{KEYWORD}必须种在同一行相邻的两株玉米投手上{NORMAL}

{FLAVOR}玉米加农炮到底是怎么回事？他上过哈佛，在纽约一家著名律师事务所执业，还能一炮炸掉整片僵尸。这些大家都知道。可在内心深处，究竟是什么推动着他？)ZH"},
    {"模仿者", R"ZH(模仿者让你在同一关里使用两张相同植物的卡牌！


{FLAVOR}“我还记得七六年的僵尸大战。”模仿者用沙哑的老年嗓音说，“那时候，可没有这么花哨的豌豆射手和火爆辣椒。我们只有胆量。胆量，还有一把勺子。”)ZH"},
    {"铁豌豆", R"ZH(铁豌豆发射沉重的铁制豌豆。

{KEYWORD}特点：{STAT}铁制弹丸，会受到磁力影响{NORMAL}

{FLAVOR}铁豌豆坚持说，自己只是吃了一顿含铁量特别高的早餐。磁力菇对此很感兴趣。他们现在保持着安全距离。)ZH"},
    {"毁灭豌豆", R"ZH(毁灭豌豆发射穿透激光，蓄力后释放收束的黑色扇形光束。

{KEYWORD}普攻：{STAT}每两秒造成200点穿透伤害{NORMAL}
{KEYWORD}技能：{STAT}每25秒蓄力释放{NORMAL}
{KEYWORD}扇形伤害：{STAT}每帧100÷当前角度{NORMAL}
{KEYWORD}终结：{STAT}仅本行造成10000点穿透伤害，无视限伤{NORMAL}
{KEYWORD}解锁：{STAT}通关支线“错误之试炼”{NORMAL}

{FLAVOR}“画面出了故障？”毁灭豌豆问，“别急着重启。那是我在工作。”他最喜欢听见的声音，是僵尸突然不说话。)ZH"},
    {"护盾坚果", R"ZH(护盾坚果替本行其他植物承受伤害，普通僵尸会直接忽略它。

{KEYWORD}耐久：{STAT}16000{NORMAL}
{KEYWORD}特点：{STAT}半透明，替本行植物受伤{NORMAL}
{KEYWORD}重击：{STAT}巨人砸击等按1800点计算{NORMAL}
{KEYWORD}弱点：{STAT}异常僵尸可直接攻击，造成原始攻击力四倍的伤害{NORMAL}

{FLAVOR}护盾坚果总说自己很有存在感。僵尸却径直从他身边走过。“没关系。”他说，“朋友们知道我在就够了。”)ZH"},
};

inline constexpr AlmanacEntryText kAlmanacZombies[] = {
    {"普通僵尸", R"ZH(花园里最常见的普通僵尸。

{KEYWORD}耐久：{STAT}低（270）{NORMAL}

{FLAVOR}这只僵尸喜欢脑子，怎么吃都吃不够。脑子，脑子，脑子，日复一日，夜复一夜。又老又臭的？腐烂的？明显过了保质期的？都无所谓。普通僵尸全都要。)ZH"},
    {"旗帜僵尸", R"ZH(旗帜僵尸标志着一大波僵尸的到来。

{KEYWORD}耐久：{STAT}低（270）{NORMAL}

{FLAVOR}别误会，旗帜僵尸也喜欢脑子。不过，不知从什么时候起，他还迷上了旗帜。也许是因为旗帜上总画着脑子。谁知道呢。)ZH"},
    {"路障僵尸", R"ZH(头上的路障让它比普通僵尸更耐打。

{KEYWORD}耐久：{STAT}中等（640）{NORMAL}

{FLAVOR}路障僵尸原本和其他僵尸一样，漫无目的地往前挪。可不知为什么，他停下来，捡起一个路障戴在头上。哦，对了。他喜欢派对。)ZH"},
    {"撑杆僵尸", R"ZH(撑杆僵尸用撑杆越过障碍。

{KEYWORD}耐久：{STAT}中等（500）{NORMAL}
{KEYWORD}速度：{STAT}快，跳跃后普通{NORMAL}
{KEYWORD}特点：{STAT}跳过遇见的第一株植物{NORMAL}

{FLAVOR}有些僵尸走得更远，追求更多，努力超越平庸，迈向伟大。这说的就是撑杆僵尸。简直就是他的写照。)ZH"},
    {"铁桶僵尸", R"ZH(头上的铁桶让它能抵挡大量伤害。

{KEYWORD}耐久：{STAT}高（1370）{NORMAL}
{KEYMETAL}弱点：{METAL}磁力菇{NORMAL}

{FLAVOR}铁桶僵尸一直戴着铁桶。一部分原因是，他想在这个冷漠的世界里彰显自己的与众不同。但主要还是因为，他压根忘了头上有个桶。)ZH"},
    {"读报僵尸", R"ZH(报纸能提供少量防护。

{KEYWORD}耐久：{STAT}低（270）{NORMAL}
{KEYWORD}报纸耐久：{STAT}低（150）{NORMAL}
{KEYWORD}速度：{STAT}普通，报纸毁坏后加快{NORMAL}

{FLAVOR}读报僵尸就差那么一点点，就能做完数独了。难怪他气得发疯。)ZH"},
    {"铁栅门僵尸", R"ZH(铁栅门是一面有效的盾牌。

{KEYWORD}耐久：{STAT}低（270）{NORMAL}
{KEYWORD}铁栅门耐久：{STAT}高（1100）{NORMAL}
{KEYWORD}弱点：{STAT}大喷菇{NORMAL}
{KEYMETAL}弱点：{METAL}磁力菇{NORMAL}

{FLAVOR}这扇铁栅门，是他从上一户防守不力的人家里拿来的。在那之前，他吃掉了屋主的脑子。)ZH"},
    {"橄榄球僵尸", R"ZH(橄榄球僵尸总能打出关键一击。

{KEYWORD}耐久：{STAT}很高（1670）{NORMAL}
{KEYWORD}速度：{STAT}快{NORMAL}
{KEYMETAL}弱点：{METAL}磁力菇{NORMAL}

{FLAVOR}橄榄球僵尸一上场就会投入百分之一百一十的努力。他有团队精神，进攻防守都很出色。他完全不知道橄榄球是什么。)ZH"},
    {"舞王僵尸", R"ZH(舞王僵尸如与任何在世或已故人物相似，纯属巧合。

{KEYWORD}耐久：{STAT}中等（500）{NORMAL}
{KEYWORD}特点：{STAT}召唤伴舞僵尸{NORMAL}

{FLAVOR}舞王僵尸的新专辑《嗷呜脑子咕噜咕噜》，正在亡灵排行榜上一路飙升。)ZH"},
    {"伴舞僵尸", R"ZH(舞王僵尸起舞时，伴舞僵尸总是四个一组登场。

{KEYWORD}耐久：{STAT}低（270）{NORMAL}

{FLAVOR}伴舞僵尸曾在僵尸纽约市的咀嚼表演艺术学院苦练六年，才把自己的舞艺磨炼到今天的水平。)ZH"},
    {"鸭子救生圈僵尸", R"ZH(鸭子救生圈让僵尸能够浮在水面上。

{KEYWORD}耐久：{STAT}低（270）{NORMAL}
{AQUATIC}仅在泳池出现{NORMAL}

{FLAVOR}不是每只僵尸都能当鸭子救生圈僵尸。有的会崩溃，受不了，转身离去，从此再也不碰脑子。)ZH"},
    {"潜水僵尸", R"ZH(潜水僵尸能够在水下游泳。

{KEYWORD}耐久：{STAT}低（270）{NORMAL}
{KEYWORD}特点：{STAT}潜入水中躲避攻击{NORMAL}
{AQUATIC}仅在泳池出现{NORMAL}

{FLAVOR}僵尸不呼吸，也不需要空气。那潜水僵尸为什么还得戴着呼吸管才能下水？答案是：同伴压力。)ZH"},
    {"冰车僵尸", R"ZH(冰车僵尸驾驶机器碾压植物，留下冰道。

{KEYWORD}耐久：{STAT}高（1350）{NORMAL}
{KEYWORD}特点：{STAT}碾压植物，留下冰道{NORMAL}

{FLAVOR}请勿将其与Zamboni®品牌的冰面整修机混淆。Zamboni®及冰面整修机形象是Frank J. Zamboni & Co., Inc.的注册商标，“Zomboni”经许可使用。如有与僵尸无关的冰面整修需求，请访问www.zamboni.com！)ZH"},
    {"雪橇僵尸小队", R"ZH(这些僵尸四个一组出现。

{KEYWORD}耐久：{STAT}每只低（270）{NORMAL}
{KEYWORD}雪橇耐久：{STAT}低（300）{NORMAL}
{KEYWORD}特点：{STAT}仅在冰道上出现{NORMAL}

{FLAVOR}雪橇僵尸小队付出了很多努力才走到今天。他们一起生活，一起吃脑子，一起训练，努力成为一支配合默契的僵尸队伍。)ZH"},
    {"海豚骑士僵尸", R"ZH(海豚骑士僵尸骑着海豚，突破你的泳池防线。

{KEYWORD}耐久：{STAT}中等（500）{NORMAL}
{KEYWORD}速度：{STAT}快，跳跃后慢{NORMAL}
{KEYWORD}特点：{STAT}跳过遇见的第一株植物{NORMAL}
{AQUATIC}仅在泳池出现{NORMAL}

{FLAVOR}那只海豚也是僵尸。)ZH"},
    {"小丑僵尸", R"ZH(这只僵尸带着一个会爆炸的惊喜。

{KEYWORD}耐久：{STAT}中等（500）{NORMAL}
{KEYWORD}速度：{STAT}快{NORMAL}
{KEYWORD}特点：{STAT}玩偶匣会爆炸{NORMAL}
{KEYMETAL}弱点：{METAL}磁力菇{NORMAL}

{FLAVOR}这只僵尸浑身发抖，不是因为冷，而是因为他疯了。)ZH"},
    {"气球僵尸", R"ZH(气球僵尸飘在战场上方，能躲过大多数攻击。

{KEYWORD}耐久：{STAT}低（270）{NORMAL}
{KEYWORD}特点：{STAT}飞行{NORMAL}
{KEYWORD}弱点：{STAT}仙人掌和三叶草{NORMAL}

{FLAVOR}气球僵尸真是走运。气球这招确实管用，而其他僵尸竟然都还没发现。)ZH"},
    {"矿工僵尸", R"ZH(矿工僵尸通过挖地道绕过你的防线。

{KEYWORD}耐久：{STAT}中等（370）{NORMAL}
{KEYWORD}速度：{STAT}先快后慢{NORMAL}
{KEYWORD}特点：{STAT}钻入地下，在草坪左侧出现{NORMAL}
{KEYWORD}弱点：{STAT}裂荚射手{NORMAL}
{KEYMETAL}弱点：{METAL}磁力菇{NORMAL}

{FLAVOR}矿工僵尸每周花三天时间，把自己的挖掘许可证整理妥当。)ZH"},
    {"跳跳僵尸", R"ZH(跳跳僵尸用跳跃绕过你的防线。

{KEYWORD}耐久：{STAT}中等（500）{NORMAL}
{KEYWORD}特点：{STAT}跳过植物{NORMAL}
{KEYMETAL}弱点：{METAL}磁力菇{NORMAL}

{FLAVOR}弹！弹！弹！这就是一只强大又高效的僵尸，正在做自己最擅长的事时发出的声音。)ZH"},
    {"雪人僵尸", R"ZH(一种罕见而奇特的生物。

{KEYWORD}耐久：{STAT}高（1350）{NORMAL}
{KEYWORD}特点：{STAT}出现不久后就会逃跑{NORMAL}

{FLAVOR}除了名字、出生日期、社会保障号码、教育经历、工作履历和三明治口味偏好——烤牛肉配瑞士奶酪——之外，我们对雪人僵尸几乎一无所知。)ZH"},
    {"飞贼僵尸", R"ZH(飞贼僵尸从上方发动攻击。

{KEYWORD}耐久：{STAT}中等（400）{NORMAL}
{KEYWORD}特点：{STAT}从天而降，偷走植物{NORMAL}

{FLAVOR}飞贼僵尸热爱冒险。毕竟，死都死了，要是不活得精彩一点，还有什么意义？)ZH"},
    {"扶梯僵尸", R"ZH(扶梯僵尸能够爬过障碍。

{KEYWORD}耐久：{STAT}中等（500）{NORMAL}
{KEYWORD}梯子耐久：{STAT}中等（500）{NORMAL}
{KEYWORD}速度：{STAT}快，架梯后慢{NORMAL}
{KEYWORD}弱点：{STAT}大喷菇{NORMAL}
{KEYMETAL}弱点：{METAL}磁力菇{NORMAL}

{FLAVOR}他花了8.99美元买到这架梯子。)ZH"},
    {"投石车僵尸", R"ZH(投石车僵尸操纵着重型机械。

{KEYWORD}耐久：{STAT}中等（850）{NORMAL}
{KEYWORD}特点：{STAT}向植物投掷篮球{NORMAL}

{FLAVOR}投石车能投的东西多了去了。可在投石车僵尸看来，篮球显然是最合适、也最理所当然的选择。)ZH"},
    {"巨人僵尸", R"ZH(巨人僵尸是一只体型庞大的僵尸。

{KEYWORD}耐久：{STAT}极高（3000）{NORMAL}

{FLAVOR}巨人僵尸走路时，大地颤抖；呻吟时，其他僵尸鸦雀无声。他是所有僵尸梦想成为的样子。可他还是找不到女朋友。)ZH"},
    {"小鬼僵尸", R"ZH(小鬼僵尸个头很小，会被巨人僵尸扔到你的防线深处。

{KEYWORD}耐久：{STAT}低（270）{NORMAL}

{FLAVOR}小鬼僵尸虽小，却很精悍。他精通僵尸柔道、僵尸空手道和僵尸徒手搏击。他还会吹口风琴。)ZH"},
    {"僵王博士", R"ZH(僵王博士统领所有僵尸。

{KEYWORD}僵王机甲内耐久：{STAT}极高{NORMAL}

{FLAVOR}埃德加·乔治·僵王只用了两年就拿到了死亡学博士学位。他迅速掌握死亡学技术，造出了可怕的僵王机甲，开始在自己所在的小区建立绝对统治。)ZH"},
    {"异常僵尸", R"ZH(异常僵尸不是独立的僵尸种类，而是可以叠加在任何僵尸身上的故障状态。它保留原本僵尸的装备、技能和攻击方式，并呈现色相偏移、横向 RGB 分离和故障抖动效果。图鉴预览每秒切换一种基础僵尸。

{KEYWORD}攻击：{STAT}攻击伤害翻倍；普通伤害每次最多造成40点{NORMAL}
{KEYWORD}特殊伤害：{STAT}毁灭豌豆的普攻激光、扇形光束和最终伤害不受限伤影响{NORMAL}
{KEYWORD}防御：{STAT}免疫魅惑、冰冻、黄油和其他控制效果，不会被普通方式再次控制{NORMAL}
{KEYWORD}护盾坚果：{STAT}可以索敌护盾坚果，并以原本攻击力的4倍伤害攻击它{NORMAL}
{KEYWORD}啃咬感染：{STAT}咬过的植物会进入故障状态，1秒后死亡；毁灭豌豆不会被感染{NORMAL}
{KEYWORD}魅惑僵尸：{STAT}啃咬魅惑僵尸时会解除魅惑，使其恢复敌对状态{NORMAL}

{FLAVOR}“我只是想修一下显示器。”一个普通僵尸说。

他按下了一个没人见过的按钮。下一秒，他的身体闪过红、绿、蓝三道影子，手里的路障却还好端端地戴在头上。“别担心，”他对旁边的僵尸说，“我感觉自己变强了。”

旁边的僵尸刚想回答，自己的脑袋也突然错开了一格。很快，整支队伍都在横向抖动，只有那个最先按下按钮的僵尸还在笑。直到他咬了一口植物，植物也开始闪烁，并在一秒后消失。

从那天起，僵尸们再也不敢修显示器了。可每当有人按错一个按钮，异常状态就会从某个僵尸身上重新亮起来。)ZH"},
    {"推车僵尸", R"ZH(推车僵尸把除草机装在头上，快速冲向植物并将它们碾压。

{KEYWORD}耐久：{STAT}低（100）{NORMAL}
{KEYWORD}速度：{STAT}普通僵尸的10倍{NORMAL}
{KEYWORD}特点：{STAT}碾压植物后短暂减速，随后恢复原速{NORMAL}
{KEYWORD}弱点：{STAT}地刺类植物抵抗碾压并造成反伤{NORMAL}
{KEYWORD}卡牌费用：{STAT}50阳光{NORMAL}

{FLAVOR}“既然除草机能清理僵尸，为什么不能清理植物？”

想到这里，他把除草机戴在了头上。其他僵尸都觉得这个主意很危险，他却只关心一件事：这次能不能第一个吃到脑子。)ZH"},
};

static_assert(sizeof(kAlmanacPlants) / sizeof(kAlmanacPlants[0]) == NUM_ALMANAC_SEEDS);
static_assert(sizeof(kAlmanacZombies) / sizeof(kAlmanacZombies[0]) == NUM_ALMANAC_ZOMBIES);
#endif
