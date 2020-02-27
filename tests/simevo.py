import math
import random
import numpy as np

max_amount = 2 ** 62 - 1

def compute(x, y, z, fee):
    prod = x * y
    tmp = -(-prod // z)  # porque // redondea siempre hacia abajo
    if (prod > 0):
        tmp_fee = (tmp * fee + 9999) // 10000
    else:
        tmp_fee =  (-tmp * fee + 9999) // 10000
    tmp += tmp_fee
    print("fee is: ",tmp_fee)
    if ( (tmp >= max_amount) or (tmp <= -max_amount) ):
        print("overflow")
    return tmp


class User:
    def __init__(self, name, e_bal, vo_bal):
        self.name = name
        self.e_bal = e_bal
        self.vo_bal = vo_bal
        self.evo_bal = 0

    def state(self):
        print(self.name, self.e_bal, self.vo_bal, self.evo_bal)

    def add_e_balance(self, x):
        if (self.e_bal + x < 0):
            print("insufficient balance")
            exit
        self.e_bal = self.e_bal + x

    def add_vo_balance(self, x):
        if (self.vo_bal + x < 0):
            print("insufficient balance")
            exit
        self.vo_bal = self.vo_bal + x

    def add_evo_balance(self, x):
        if (self.evo_bal + x < 0):
            print("insufficient balance")
            exit
        self.evo_bal = self.evo_bal + x


class Evodex:
    def __init__(self, user, e_bal, vo_bal, evo_minted, fee):
        user.add_e_balance(-e_bal)
        user.add_vo_balance(-vo_bal)
        user.add_evo_balance(evo_minted)
        self.fee = fee
        self.e_bal = e_bal
        self.vo_bal = vo_bal
        self.evo_minted = evo_minted
        self.state_parameter = pow(self.e_bal, 0.5) * pow(self.vo_bal, 0.5) / self.evo_minted
    
#    def state_parameter(self):
#        return pow(self.e_bal, 0.5) * pow(self.vo_bal, 0.5) // self.evo_minted

    def changefee(self, newfee):
        self.fee = newfee

    def buytoken(self, user, amount):
        if (amount < 0):
            print("input must be positive")
            return
        print(user.name, "buytoken", amount)
        self.buyorsell(user, amount, self.fee)

    def selltoken(self, user, amount):
        if (amount < 0):
            print("input must be positive")
            return
        print(user.name, "selltoken", amount)
        self.buyorsell(user, -amount, 0)

    def buyorsell(self, user, amount, fee):
        A = self.evo_minted
        C1 = self.e_bal
        C2 = self.vo_bal

        to_pay_e = compute(amount, C1, A, fee)
        to_pay_vo = compute(amount, C2, A, fee)
        user.add_e_balance(-to_pay_e)
        user.add_vo_balance(-to_pay_vo)
        user.add_evo_balance(amount)
        print(user.name, "delta: ", -to_pay_e, -to_pay_vo, amount)
        self.e_bal += to_pay_e
        self.vo_bal += to_pay_vo
        self.evo_minted += amount

    def exchange(self, user, e, vo):
        print(user.name, "exchange", e, vo)
        C1 = self.e_bal
        C2 = self.vo_bal
        vo_out = compute(-e, C2, C1+e, self.fee)
        if (vo_out > vo):
            print("available is less than expected")
            return
        print(user.name, "delta: ", -e, -vo_out, 0)
        user.add_e_balance(-e)
        user.add_vo_balance(-vo_out)

        self.e_bal += e
        self.vo_bal += vo_out
        print('vo_out:', vo_out)


    def state(self):
        print("Evodex: ", self.e_bal, self.vo_bal, self.evo_minted)
        self.state_parameter =  pow(self.e_bal, 0.5) * pow(self.vo_bal, 0.5) / self.evo_minted
        print("Characteristic number:", self.state_parameter, "\n") # pow(self.e_bal, self.e_weight) * pow(self.vo_bal, self.vo_weight) / self.evo_minted, "\n" )
