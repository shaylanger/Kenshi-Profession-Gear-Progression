#!/usr/bin/env python3
"""PG 199 ranged Perception gate: evaluate the kah CSV of tests/ingame/full-base/pg-95-ranged-perception.txt.

Usage:  python tools/ranged_gate.py <pg95.csv>      one line per point:
            RESULT 199a|199b|199c PASS|FAIL <evidence>
        python tools/ranged_gate.py --selftest      offline check on built-in synthetic samples

Kenshi GunClass::shoot: acc01 = 0.005*Crossbows + 0.005*Perception (both CharStats::getStat, PG-hooked),
dev = dev_base*(1-acc01). Crossbows fixed at 50 -> expected acc01 a .375 (Perception 25), b .70 (90),
c .4375 (25 * 1.5 gear); mean_dev ratios b/a .48, c/a .90. Hit rate should rise a -> c -> b, but it is
noisy (20 shots): a wrong direction is reported (hit_dir=noise), never a FAIL when acc01/spread match.
Stdlib only (Windows `python`); exit 0 when all three points pass.
"""
import csv
import re
import sys

EXPECT_ACC = {'199a': 0.375, '199b': 0.70, '199c': 0.4375}
ACC_TOL = 0.02          # Perception/Crossbows XP while shooting moves acc01 a little
EXPECT_RATIO = {'199b': 0.48, '199c': 0.90}
RATIO_TOL = 0.04
PERC_BAND = {'199a': (24.9, 27.5), '199b': (89.9, 91.5), '199c': (37.3, 41.0)}
WEAPON_BAND = (49.9, 51.5)
PROBE_KEY = '0x874b84'  # return address 0x43A5C2 (Perception getStat in GunClass::shoot) * 2 + raw(0)


def kv(text):
    out = {}
    for k, v in re.findall(r'(\w+)=(\S+)', text or ''):
        out.setdefault(k, v)
    return out


def num(d, k, default=None):
    try:
        return float(d[k])
    except (KeyError, ValueError, TypeError):
        return default


def parse(rows):
    """rows: dicts with line/result/step/detail. Returns {point: info}."""
    points = {}
    cur = None
    for r in rows:
        step = (r.get('step') or '').strip()
        detail = r.get('detail') or ''
        ok = r.get('result') == 'PASS'
        m = re.match(r'@echo POINT (199[abc])\b', step)
        if m:
            cur = m.group(1)
            points[cur] = {'session': None, 'session_ok': False, 'probe': [], 'bonus': [], 'fails': []}
            continue
        if cur is None:
            continue
        p = points[cur]
        if not ok and not step.startswith('@until') and not step.startswith('rangedtest Beaks last'):
            p['fails'].append('line%s' % r.get('line'))
        if re.match(r'@set R[ABC] rangedtest ', step):
            reply = detail.split(' | ', 1)[1] if re.match(r'R[ABC]=', detail) and ' | ' in detail else detail
            head, _, last = reply.partition('| last:')
            p['session'] = {'head': kv(head), 'last': kv(last), 'raw': reply}
            p['session_ok'] = ok
        elif step.startswith('pg_statprobe callers ~'):
            m2 = re.search(re.escape(PROBE_KEY) + r':(\d+)', detail)
            p['probe'].append(int(m2.group(1)) if (ok and m2) else 0)
        elif step.startswith('pg_bonus Beaks perception'):
            b = kv(detail)
            p['bonus'].append((ok, b.get('equipped_bonus'), b.get('match')))
    return points


def evaluate(points):
    lines = []
    allpass = True
    a_dev = None
    a_hit = None
    sa = (points.get('199a') or {}).get('session')
    if sa:
        a_dev = num(sa['head'], 'mean_dev')
        a_hit = num(sa['head'], 'hit_rate')
    for pt in ('199a', '199b', '199c'):
        p = points.get(pt)
        why = []
        ev = ''
        if not p:
            lines.append('RESULT %s FAIL point not run (no "@echo POINT %s" row)' % (pt, pt))
            allpass = False
            continue
        s = p['session']
        if not s:
            why.append('no rangedtest session row')
            h, last = {}, {}
        else:
            h, last = s['head'], s['last']
        shots = int(num(h, 'shots', 0) or 0)
        hits = int(num(h, 'hits', 0) or 0)
        hit_rate = num(h, 'hit_rate')
        acc = num(h, 'mean_acc01')
        dev = num(h, 'mean_dev')
        devf = num(h, 'mean_dev_formula')
        perc = num(h, 'mean_perception_eff')
        weap = num(h, 'mean_weapon_eff')
        complete = h.get('complete', '?')
        probe = max(p['probe']) if p['probe'] else 0
        if shots <= 0:
            why.append('shots=0')
        if acc is None or abs(acc - EXPECT_ACC[pt]) > ACC_TOL:
            why.append('acc01=%s want %.4f' % (acc, EXPECT_ACC[pt]))
        if dev is None or devf is None or abs(dev - devf) > max(0.002, 0.02 * abs(devf)):
            why.append('dev %s != formula %s' % (dev, devf))
        lo, hi = PERC_BAND[pt]
        if perc is None or not lo <= perc <= hi:
            why.append('perception_eff=%s want %g-%g' % (perc, lo, hi))
        if weap is None or not WEAPON_BAND[0] <= weap <= WEAPON_BAND[1]:
            why.append('weapon_eff=%s want 50-51.5' % weap)
        if probe <= 0:
            why.append('statprobe %s missing' % PROBE_KEY)
        ratio = None
        hit_dir = '-'
        if pt != '199a':
            if a_dev and dev is not None:
                ratio = dev / a_dev
                if abs(ratio - EXPECT_RATIO[pt]) > RATIO_TOL:
                    why.append('dev ratio %.3f want %.2f' % (ratio, EXPECT_RATIO[pt]))
                if not dev < a_dev:
                    why.append('spread not below 199a')
            else:
                why.append('no 199a spread to compare')
            if hit_rate is not None and a_hit is not None:
                hit_dir = 'ok' if hit_rate >= a_hit else 'noise'
        want_bonus = '50%' if pt == '199c' else '0%'
        if not p['bonus']:
            why.append('no pg_bonus row')
        for ok, eb, match in p['bonus']:
            if not ok or eb != want_bonus or match != '1':
                why.append('pg_bonus equipped_bonus=%s match=%s want %s/1' % (eb, match, want_bonus))
                break
        if pt == '199c':
            pb, pe = num(last, 'perception_base'), num(last, 'perception_eff')
            if not pb or pe is None or abs(pe / pb - 1.5) > 0.03:
                why.append('last perception_eff/base=%s/%s want x1.5' % (pe, pb))
        if p['fails']:
            ev_fail = ' step_fails=' + ','.join(p['fails'][:6])
        else:
            ev_fail = ''
        ev = ('shots=%d hits=%d hit_rate=%s complete=%s mean_acc01=%s mean_dev=%s dev_formula=%s '
              'perception_eff=%s weapon_eff=%s probe=%s:%d' %
              (shots, hits, hit_rate, complete, acc, dev, devf, perc, weap, PROBE_KEY, probe))
        if ratio is not None:
            ev += ' dev_ratio=%.3f hit_dir=%s' % (ratio, hit_dir)
        if pt == '199c':
            ev += ' bonus=%s' % (p['bonus'][0][1] if p['bonus'] else '?')
        ev += ev_fail
        ok = not why
        allpass &= ok
        lines.append('RESULT %s %s %s%s' % (pt, 'PASS' if ok else 'FAIL', ev, '' if ok else ' | ' + '; '.join(why)))
    return lines, allpass


def read_csv(path):
    with open(path, newline='', encoding='utf-8') as f:
        return list(csv.DictReader(f))


def sample(acc_c=0.4375, perc_c=37.5, probe_c=12, bonus_c='50%'):
    def sess(tag, acc, dev, perc, hit, pb, pe):
        reply = ('Beaks rangedtest target=PGR%s #9 complete=1 shots=20 hits=%d misses=%d pending=0 hit_rate=%.4f '
                 'other_damage=0 mean_acc01=%.4f mean_dev=%.4f mean_dev_formula=%.4f min_dev=%.4f max_dev=%.4f '
                 'mean_weapon_eff=50.2000 mean_perception_eff=%.4f mean_dist=40 heals=5 aim_sets=1 seconds=60 '
                 'real_seconds=61 | last: stat=35 gun=personal weapon_base=50.0000 weapon_eff=50.4000 '
                 'perception_base=%.4f perception_eff=%.4f acc01=%.4f dev=%.4f dev_formula=%.4f dev_base=0.1200 '
                 'range=500 dist=40 target=PGR%s #9' %
                 (tag, int(hit * 20), 20 - int(hit * 20), hit, acc, dev, dev, dev, dev, perc, pb, pe, acc, dev, dev,
                  tag))
        head = reply.split(' complete=', 1)[1]
        return {'line': '1', 'result': 'PASS', 'step': '@set R%s rangedtest Beaks PGR%s shots 20 timeout 400 attack'
                ' ~ (complete=\\d shots=[1-9].*)' % (tag, tag), 'detail': 'R%s=complete=%s | %s' % (tag, head, reply)}

    def row(step, detail, result='PASS'):
        return {'line': '1', 'result': result, 'step': step, 'detail': detail}

    def bonus(b, match='1'):
        return row('pg_bonus Beaks perception ~ equipped_bonus=%s .*match=1' % b,
                   'Beaks Perception equipped_bonus=%s base=25.00 vanilla_effective=25.00 effective=25.00 '
                   'expected=25.00 match=%s' % (b, match))

    def probe(n):
        return row('pg_statprobe callers ~ 0x874b84:[1-9]',
                   'statprobe on: Perception=mod:%d,raw:0 callers(rva*2+raw)=0x10a0:3,0x874b84:%d' % (n, n),
                   'PASS' if n else 'FAIL')

    base = 0.12
    rows = [row('@echo POINT 199a perception=25 gear=none', 'POINT 199a perception=25 gear=none'),
            bonus('0%'), sess('A', 0.375, base * 0.625, 25.2, 0.30, 25, 25.2), probe(20),
            row('@echo POINT 199b perception=90 gear=none', 'POINT 199b perception=90 gear=none'),
            bonus('0%'), sess('B', 0.70, base * 0.30, 90.1, 0.55, 90, 90.1), probe(20),
            row('@echo POINT 199c perception=25 gear=perception50', 'POINT 199c'),
            bonus(bonus_c), sess('C', acc_c, base * (1 - acc_c), perc_c, 0.25, 25, perc_c), probe(probe_c)]
    return rows


def selftest():
    fails = 0

    def check(name, rows, want):
        nonlocal fails
        lines, allpass = evaluate(parse(rows))
        got = [l.split()[2] for l in lines]
        good = got == want
        fails += not good
        print('%s selftest %s: %s' % ('ok  ' if good else 'FAIL', name, ' '.join(got)))
        if not good:
            for l in lines:
                print('     ' + l)

    check('good run (hit-rate noise on c tolerated)', sample(), ['PASS', 'PASS', 'PASS'])
    check('gear not applied (c = a)', sample(acc_c=0.375, perc_c=25.0, bonus_c='0%'), ['PASS', 'PASS', 'FAIL'])
    check('no statprobe on c', sample(probe_c=0), ['PASS', 'PASS', 'FAIL'])
    rows = sample()
    rows[2] = dict(rows[2], result='FAIL', detail='no match for /x/: Beaks rangedtest target=PGRA #9 complete=0 '
                   'shots=0 hits=0 misses=0 pending=0 hit_rate=0 mean_acc01=0 mean_dev=0 mean_dev_formula=0 '
                   'mean_perception_eff=0 mean_weapon_eff=0 | last:')
    check('no shots on a', rows, ['FAIL', 'FAIL', 'FAIL'])
    check('point c missing', sample()[:8], ['PASS', 'PASS', 'FAIL'])
    print('selftest %s' % ('PASS' if not fails else 'FAIL (%d)' % fails))
    return 1 if fails else 0


def main(argv):
    if not argv or argv[0] in ('-h', '--help'):
        print(__doc__)
        return 2
    if argv[0] == '--selftest':
        return selftest()
    lines, allpass = evaluate(parse(read_csv(argv[0])))
    for l in lines:
        print(l)
    return 0 if allpass else 1


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
