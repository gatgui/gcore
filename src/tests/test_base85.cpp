/*

Copyright (C) 2014~  Gaetan Guidet

This file is part of gcore.

gcore is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or (at
your option) any later version.

gcore is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
USA.

*/

#include <gcore/gcore>

int main(int argc, char **argv) {
  
  std::map<std::string, int> csid;
  std::map<std::string, int>::iterator it;
  
  //float  indata[6] = {1.0f, 2.0f, 3.0f, -3.0f, -2.0f, -1.0f};
  //float  indata[6] = {2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f};
  //float  indata[6] = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
  //float  indata[6] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
  float  indata[15] = {0.0f, 0.1f, 0.1f, 0.1f, 0.2f, 0.2f, 0.2f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
  size_t inlen      = 15 * sizeof(float);
  void  *outdata    = 0;
  size_t outlen     = 0;
  
  csid["ascii85"] = gcore::Base85::Ascii85;
  csid["z85"] = gcore::Base85::Z85;
  csid["ipv6"] = gcore::Base85::IPV6;
  
  int cs = gcore::Base85::Ascii85;
  std::string encstr = "";
  std::string decstr = "";
  
  for (int a=1; a<argc; ++a) {
    if (!strcmp(argv[a], "-cs")) {
      ++a;
      if (a >= argc) {
        std::cout << "invalid arguments (-cs ascii85|z85|ipv6, -e <str> or -d <str> allowed)" << std::endl;
        return 1;
      }
      it = csid.find(argv[a]);
      if (it != csid.end()) {
        cs = it->second;
      } else {
        std::cout << "No such charset \"" << argv[a] << "\", use acii85" << std::endl;
      }
    
    } else if (!strcmp(argv[a], "-e")) {
      ++a;
      if (a >= argc) {
        std::cout << "invalid arguments (-cs ascii85|z85|ipv6, -e <str> or -d <str> allowed)" << std::endl;
        return 1;
      }
      encstr = argv[a];
    
    } else if (!strcmp(argv[a], "-d")) {
      ++a;
      if (a >= argc) {
        std::cout << "invalid arguments (-cs ascii85|z85|ipv6, -e <str> or -d <str> allowed)" << std::endl;
        return 1; 
      }
      decstr = argv[a];
    
    } else {
      std::cout << "skip argument \"" << argv[a] << "\"" << std::endl;
    }
    
  }
  
  gcore::Base85::Encoder *enc = gcore::Base85::CreateEncoder(cs);
  gcore::Base85::Decoder *dec = gcore::Base85::CreateDecoder(cs);
  
  // test 1
  std::string es, ds;
  
  es = gcore::Base85::Encode(enc, &indata[0], inlen);
  
  std::cout << "Encoded floats: \"" << es << "\"" << std::endl;
  
  std::cout << es << std::endl;
  if (gcore::Base85::Decode(dec, es, outdata, outlen)) {
    float *f = (float*)outdata;
    size_t n = outlen / 4;
    std::cout << "-> [" << outlen << " bytes(s)] ";;
    for (size_t i=0; i<n; ++i) {
       std::cout << f[i] << " ";
    }
    std::cout << std::endl;
    free(outdata);
    
  } else {
    std::cerr << "Could not decode" << std::endl;
  }
  
  // custom encoding / decoding tests
  if (encstr.length() > 0) {
    std::cout << "=== Encode \"" << encstr << "\"..." << std::endl;
    
    //es = gcore::Base85::Encode(encstr, cs);
    es = gcore::Base85::Encode(enc, encstr);
    std::cout << "\"" << encstr << "\" => \"" << es << "\" (" << es.length() << ")" << std::endl;
     
    if (gcore::Base85::Decode(dec, es, ds)) {
      std::cout << "\"" << es << "\" => \"" << ds << "\" (" << ds.length() << ")" << std::endl;
    }
  }
  
  if (decstr.length() > 0) {
    std::cout << "=== Decode \"" << decstr << "\"..." << std::endl;
    
    if (!gcore::Base85::Decode(dec, decstr, ds)) {
      std::cout << "Failed" << std::endl;
    
    } else {
      std::cout << "\"" << decstr << "\" => - ";
      for (size_t i=0; i<ds.length(); ++i) {
        std::cout << std::hex << int((unsigned char)ds[i]) << std::dec << " - ";
      }
      std::cout << std::endl;
      
      es = gcore::Base85::Encode(enc, ds);
      std::cout << "=> \"" << es << "\"" << std::endl;
    }
  }
  
  gcore::Base85::DestroyEncoder(enc);
  gcore::Base85::DestroyDecoder(dec);
  
  // Custom encoding test
  const gcore::Base85::Encoding *ascii85 = gcore::Base85::GetEncoding(gcore::Base85::Ascii85);
  
  std::string charset = ascii85->charset + 3;
  charset += "vwx";
  
  gcore::Base85::Encoding e_pack4;
  e_pack4.charset = charset.c_str();
  e_pack4.rlemarker = '!';
  e_pack4.revbytes = true;
  e_pack4.pack = 4;
  e_pack4.specials['z'] = 0x00000000;
  e_pack4.specials['y'] = 0x3F800000;
  gcore::Base85::AddEncoding("pack4", &e_pack4);
  
  gcore::Base85::Encoding e_pack3 = e_pack4;
  e_pack3.pack = 3;
  gcore::Base85::AddEncoding("pack3", &e_pack3);
  
  gcore::Base85::Encoding e_pack2 = e_pack4;
  e_pack2.pack = 2;
  gcore::Base85::AddEncoding("pack2", &e_pack2);
  
  gcore::Base85::Encoding e_pack1 = e_pack2;
  e_pack1.pack = 1;
  gcore::Base85::AddEncoding("pack1", &e_pack1);
  
  enc = gcore::Base85::CreateEncoder("pack4");
  dec = gcore::Base85::CreateDecoder("pack4");
  
  std::cout << "=== UINT arrays test 1/4" << std::endl;
  outdata = 0;
  outlen = 0;
  // 256 uints, values in [0-255] range
  decstr = "!%<_l4$$$$d";
  if (gcore::Base85::Decode(dec, decstr, outdata, outlen)) {
     unsigned int *vals = (unsigned int*)outdata;
     unsigned int count = outlen / 4;
     std::cout << "Decoded " << count << " values" << std::endl;
     std::cout << "  [0] " << vals[0] << std::endl;
     std::cout << "  [1] " << vals[1] << std::endl;
     std::cout << "  [2] " << vals[2] << std::endl;
     std::cout << "  ..." << std::endl;
     std::cout << "  [" << (count / 2) << "] " << vals[count/2] << std::endl;
     std::cout << "  ..." << std::endl;
     std::cout << "  [" << (count - 3) << "] " << vals[count-3] << std::endl;
     std::cout << "  [" << (count - 2) << "] " << vals[count-2] << std::endl;
     std::cout << "  [" << (count - 1) << "] " << vals[count-1] << std::endl;
     
     es = gcore::Base85::Encode(enc, outdata, outlen);
     std::cout << "=> Re-encoded to: " << es << std::endl;
     if (es != decstr) {
       std::cout << "!!! DIFFERS FROM ORIGINAL !!!" << std::endl;
     }
     
     free(outdata);
  }
   
  std::cout << "=== UINT arrays test 2/4" << std::endl;
  outdata = 0;
  outlen = 0;
  // 25 uints, values in [0-255] range
  decstr = "!%<_l4$$$$'$vDc3$v;Z0$v;Z0$u";
  if (gcore::Base85::Decode(dec, decstr, outdata, outlen)) {
    unsigned int *vals = (unsigned int*)outdata;
    unsigned int count = outlen / 4;
    std::cout << "Decoded " << count << " values" << std::endl;
    for (unsigned int i=0; i<count; ++i) {
      std::cout << "  [" << i << "] " << vals[i] << std::endl;
    }
    
    es = gcore::Base85::Encode(enc, outdata, outlen);
    std::cout << "=> Re-encoded to: " << es << std::endl;
    if (es != decstr) {
      std::cout << "!!! DIFFERS FROM ORIGINAL !!!" << std::endl;
    }
    
    free(outdata);
  }
  
  std::cout << "=== UINT arrays test 3/4" << std::endl;
  outdata = 0;
  outlen = 0;
  // 66049 uints, values in [0-255] range
  decstr = "!%<_l4$$&<:%<";
  if (gcore::Base85::Decode(dec, decstr, outdata, outlen)) {
    unsigned int *vals = (unsigned int*)outdata;
    unsigned int count = outlen / 4;
    std::cout << "Decoded " << count << " values" << std::endl;
    std::cout << "  [0] " << vals[0] << std::endl;
    std::cout << "  [1] " << vals[1] << std::endl;
    std::cout << "  [2] " << vals[2] << std::endl;
    std::cout << "  ..." << std::endl;
    std::cout << "  [" << (count / 2) << "] " << vals[count/2] << std::endl;
    std::cout << "  ..." << std::endl;
    std::cout << "  [" << (count - 3) << "] " << vals[count-3] << std::endl;
    std::cout << "  [" << (count - 2) << "] " << vals[count-2] << std::endl;
    std::cout << "  [" << (count - 1) << "] " << vals[count-1] << std::endl;
    
    es = gcore::Base85::Encode(enc, outdata, outlen);
    std::cout << "=> Re-encoded to: " << es << std::endl;
    if (es != decstr) {
      std::cout << "!!! DIFFERS FROM ORIGINAL !!!" << std::endl;
    }
    
    free(outdata);
  }
  
  gcore::Base85::DestroyEncoder(enc);
  gcore::Base85::DestroyDecoder(dec);
  
  dec = gcore::Base85::CreateDecoder("pack3");
  enc = gcore::Base85::CreateEncoder("pack3");
  
  std::cout << "=== UINT arrays test 4/4" << std::endl;
  outdata = 0;
  outlen = 0;
  // 1024 uints, indices with value in [0, 288] range
  decstr = "8s0=4$'GFY8WlW5$DL@S$Gm@3$,R7C$,TZ4$IW1=9(Es]$1](-$1_Js9*/dg$R-v\\"
           "8g5[W$6j;]$Slgf92[U18l@LA$;u,G94EF;$\\CX0$BdfT$Bg4F$_i`O$c5`/$GoW?$"
           "Gr%0$dtQ9$h@Pn$M%H)$M'jo$j*Ax$mKAX9-S&S$R2[Y$o52b9Mxu-$W;)R$W=LC9Ob"
           "f79S.el$\\Eo<$^/H=9VTn69Yumk9>Ze&$c:E,%+<q59_+^U9CeUe$hE5k%0Gat%3ha"
           "T9HpFO9Hri@%5RR^%8sR>$rXIN$rZl?9k+13%>)C($wc:8$we])9p5vr9u>8g%)UB7%"
           ")We(%FZ<1:%I)Q9_-ua%.bUg%Ke,p:*So;9d8fK%3mFQ%PorZ%T;r:9iCW59iF%&%V"
           "%cD%YFc$%>+Z4%>.(%:6SAn%^QSc%Drb2%Du0$:=EIm:@fIM%J(Rr%J*uc%g-Ll:Eq:"
           "7:*V1G%O5fM:G[+A%oY=6%T>4F%T@W7:Leq+:P1p`%YI%0%YKGv%vMt*&$ns_:9vXZ:"
           ";`1[&(@')&+a&^%eErn%eH@_&-Jlh:a9Z3%jPcX%jS1I&2U]R&5v]2%o[TB:K+ds&7`"
           "N<&;,Mq%tfE,%thgr&<k?&:pZ,F:U>xV&$sX\\&Av/e&G)FZ:\\1+U:\\3NF;$6%O&L"
           "47D&0n.T&0pQE&Ms(N&Q?(.:fFb)&6&B/&S(n8&VImm&;.e(&;12n;3VLb&[T^W&@9U"
           "g&@;xX&]>Oa&`_OA:wNKP:wPnB;?SEK;BtE+;'Y<;;'[_,;D^65;H*5j;,d-%&QCb+;"
           "Ii&t;M5&T;1nrd;1q@U;Nsl^;R?l>&[Vuc&[YCT;T)]H;WJ](;</T8&bK?N'*Mw\\'-"
           "nw<&gSnL&gV<='/XhF'3$h&&l^_6&la-''4cY0'8/Xe&qiOu&qkrf'9nIo'=:IO&vt@"
           "_&vvcP'?$:Y'BE:9''*1I'',T:'D/+C'I7B8'-q9H'-s\\9'Jv3B'NB2w'3'*2'3)Lx"
           "'P,$,'SLxa'81oq'84=b'U6ik'XWiK'=<`['=?.L<5dH@<90Gu;rj?0;rlav'_LK?<>"
           ";8_<$\\G.<$^iu<AaA)<E-@^<)g7n'NFlt'kID(<J81H<.r(X<.tKI<KvwR<OBw2<4'"
           "nB'X\\NH'u_%Q($+%1']dqA']g?2(%ik;()5jp'bob+'dY;,(,[s:(0'ro'iaj*'id7"
           "p(1fd$(52cY'nlZi'no(Z(6qTc(:=TC'swKS't$nD(<'EM(?HE-($-<=($/_.(A267<"
           "tuxW()8-'():Om(F=&v(KE=k(0*5&(0,Wl(M/.u(PP.U(55%e(57HV(R9t_(UZt?(:?"
           "kO(:B9@(WDeI(Zee)(?J\\9(?M**(\\OV3(_pUh(DULx(DWoi(aZFr(e&FR(KGTv(KI"
           "wh(hLNq(kmNQ(PREa(PThR(mW?[(px?;(U]6K(U_Y<(rb0E(v.0%(Zh'5(ZjJ&(wlv/"
           ")&8ud(_rlt(_u:e)'wfn)+CfN(e(]^(fg6_).inm)25nM(koe](kr3N)3t_W)7@_7(q"
           "%VG(q($8)9*PA)<KOv(v0G1(v2iw)>5A+)AV@`)&;7p)&=Za)C@1j)Fa1J)+F(Z)+HK"
           "K)HJwT)MS9I)280Y)2:SJ)O=*S)R^*3)7BvC)7ED4)TGp=)Whor)<Mg-)<P4s)YRa')"
           "\\s`\\)AXWl)A[%])^]Qf)b)QF)FcHV)FekG)chBP)g4B0$$$'D";
  // Should start with: 0, 1, 18, 17, 1, 2, 19, 18, 2, 3, 20, 19, ...
  // => Count is not correct (1026 instead of expected 1024, values are correct)
  if (gcore::Base85::Decode(dec, decstr, outdata, outlen)) {
    unsigned int *vals = (unsigned int*)outdata;
    unsigned int count = outlen / 4;
    std::cout << "Decoded " << count << " indices" << std::endl;
    std::cout << "  [0] " << vals[0] << std::endl;
    std::cout << "  [1] " << vals[1] << std::endl;
    std::cout << "  [2] " << vals[2] << std::endl;
    std::cout << "  [3] " << vals[3] << std::endl;
    std::cout << "  ..." << std::endl;
    std::cout << "  [" << (count / 2) << "] " << vals[count/2] << std::endl;
    std::cout << "  ..." << std::endl;
    std::cout << "  [" << (count - 6) << "] " << vals[count-6] << std::endl;
    std::cout << "  [" << (count - 5) << "] " << vals[count-5] << std::endl;
    std::cout << "  [" << (count - 4) << "] " << vals[count-4] << std::endl;
    std::cout << "  [" << (count - 3) << "] " << vals[count-3] << std::endl;
    std::cout << "  [" << (count - 2) << "] " << vals[count-2] << std::endl;
    std::cout << "  [" << (count - 1) << "] " << vals[count-1] << std::endl;
    
    es = gcore::Base85::Encode(enc, outdata, outlen);
    std::cout << "=> Re-encoded to: " << es << std::endl;
    if (es != decstr) {
      // Expected: The difference is in the higher 2 bits of each 4 bytes chunk
      //           which are actually ignored by the pack3 10 bits mask
      //           Data is actually perfectly identical
      std::cout << "!!! DIFFERS FROM ORIGINAL !!!" << std::endl;
    }
    
    free(outdata);
  }
  
  gcore::Base85::DestroyDecoder(dec);
  gcore::Base85::DestroyEncoder(enc);
  
  dec = gcore::Base85::CreateDecoder("pack2");
  enc = gcore::Base85::CreateEncoder("pack2");
  
  outdata = 0;
  outlen = 0;
  // 10000 uints, indices with value in [0, 2600]
  decstr = "$$-*%$)IX6$$60'$)R^8$$?6)$)[d:$$H<+$)dj<$$QB-$)mp>$$ZH/$)vv@$$cN1$"
           "*+'B$$lT3$*4-D$$uZ5$*=3F$%)`7$*F9H$%2f9$*O?J$%;l;$*XEL$%Dr=$*aKN$%"
           "Mx?$*jQP$%W)A$*sWR$%`/C$+']T$%i5E$+0cV$%r;G$+9iX$&&AI$+BoZ$&/GK$+K"
           "u\\$&8MM$+U&^$&ASO$+^,`$&JYQ$+g2b$&S_S$+p8d$&\\eU$,$>f$&ekW$,-Dh$&"
           "nqY$,6Jj$&ww[$,?Pl$',(]$,HVn$'5._$,Q\\p$'>4a$,Zbr$'G:c$,cht$'P@e$,l"
           "nv$'YFg$,utx$'bLi$-*&%$'kRk$-3,'$'tXm$-<2)$((^o$-E8+$(1dq$-N>-$(:js"
           "$-WD/$(Cpu$-`J1$(Lvw$-iP3$(V($$-rV5$(_.&$.&\\7$(h4($./b9$(q:*$.8h;$"
           ")%@,$.An=$).F.$.Jt?$)7L0$.T%A$)@R2$.]+C$)R^6$.o7G$)[d8$.x=I$)dj:$/"
           ",CK$)mp<$/5IM$)vv>$/>OO$*+'@$/GUQ$*4-B$/P[S$*=3D$/YaU$*F9F$/bgW$*O?"
           "H$/kmY$*XEJ$/ts[$*aKL$0)$]$*jQN$02*_$*sWP$0;0a$+']R$0D6c$+0cT$0M<e$"
           "+9iV$0VBg$+BoX$0_Hi$+KuZ$0hNk$+U&\\$0qTm$+^,^$1%Zo$+g2`$1.`q$+p8b$1"
           "7fs$,$>d$1@lu$,-Df$1Irw$,6Jh$1S$$$,?Pj$1\\*&$,HVl$1e0($,Q\\n$1n6*$,"
           "Zbp$1w<,$,chr$2+B.$,lnt$24H0$,utv$2=N2$-*%x$2FT4$-3,%$2OZ6$-<2'$2X`"
           "8$-E8)$2af:$-N>+$2jl<$-WD-$2sr>$-`J/$3'x@$-iP1$31)B$-rV3$3:/D$.&\\5"
           "$3C5F$./b7$3L;H$.8h9$3UAJ$.An;$3^GL$.Jt=$3gMN$.T%?$3pSP$.]+A$4$YR$."
           "f1C$4-_T$.x=G$4?kX$/,CI$4HqZ$/5IK$4Qw\\$/>OM$4[(^$/GUO$4d.`$/P[Q$4m"
           "4b$/YaS$4v:d$/bgU$5*@f$/kmW$53Fh$/tsY$5<Lj$0)$[$5ERl$02*]$5NXn$0;0_"
           "$5W^p$0D6a$5`dr$0M<c$5ijt$0VBe$5rpv$0_Hg$6&vx$0hNi$60(%$0qTk$69.'$1"
           "%Zm$6B4)$1.`o$6K:+$17fq$6T@-$1@ls$6]F/$1Iru$6fL1$1Rxw$6oR3$1\\*$$6x"
           "X5$1e0&$7,^7$1n6($75d9$1w<*$7>j;$2+B,$7Gp=$24H.$7Pv?$2=N0$7Z'A$2FT2"
           "$7c-C$2OZ4$7l3E$2X`6$7u9G$2af8$8)?I$2jl:$82EK$2sr<$8;KM$3'x>$8DQO$3"
           "1)@$8MWQ$3:/B$8V]S$3C5D$8_cU$3L;F$8hiW$3UAH$8qoY$3^GJ$9%u[$3gML$9/&"
           "]$3pSN$98,_$4$YP$9A2a$4-_R$9J8c$46eT$9S>e$4HqX$9eJi$4QwZ$9nPk$4[(\\"
           "$9wVm$4d.^$:+\\o$4m4`$:4bq$4v:b$:=hs$5*@d$:Fnu$53Ff$:Otw$5<Lh$:Y&$$"
           "5ERj$:b,&$5NXl$:k2($5W^n$:t8*$5`dp$;(>,$5ijr$;1D.$5rpt$;:J0$6&vv$;C"
           "P2$60'x$;LV4$69.%$;U\\6$6B4'$;^b8$6K:)$;gh:$6T@+$;pn<$6]F-$<$t>$6fL"
           "/$<.%@$6oR1$<7+B$6xX3$<@1D$7,^5$<I7F$75d7$<R=H$7>j9$<[CJ$7Gp;$<dIL$"
           "7Pv=$<mON$7Z'?$<vUP$7c-A$=*[R$7l3C$=3aT$7u9E$=<gV$8)?G$=EmX$82EI$=N"
           "sZ$8;KK$=X$\\$8DQM$=a*^$8MWO$=j0`$8V]Q$=s6b$8_cS$>'<d$8hiU$>0Bf$8qo"
           "W$>9Hh$9%uY$>BNj$9/&[$>KTl$98,]$>TZn$9A2_$>]`p$9J8a$>ffr$9S>c$>olt$"
           "9\\De$>xrv$9nPi$?6*%$9wVk$??0'$:+\\m$?H6)$:4bo$?Q<+$:=hq$?ZB-$:Fns$"
           "?cH/$:Otu$?lN1$:Y%w$?uT3$:b,$$@)Z5$:k2&$@2`7$:t8($@;f9$;(>*$@Dl;$;1"
           "D,$@Mr=$;:J.$@Vx?$;CP0$@`)A$;LV2$@i/C$;U\\4$@r5E$;^b6$A&;G$;gh8$A/A"
           "I$;pn:$A8GK$<$t<$AAMM$<.%>$AJSO$<7+@$ASYQ$<@1B$A\\_S$<I7D$AeeU$<R=F"
           "$AnkW$<[CH$AwqY$<dIJ$B+w[$<mOL$B5(]$<vUN$B>._$=*[P$BG4a$=3aR$BP:c$="
           "<gT$BY@e$=EmV$BbFg$=NsX$BkLi$=X$Z$BtRk$=a*\\$C(Xm$=j0^$C1^o$=s6`$C:"
           "dq$>'<b$CCjs$>0Bd$CLpu$>9Hf$CUvw$>BNh$C_($$>KTj$Ch.&$>TZl$Cq4($>]`n"
           "$D%:*$>ffp$D.@,$>olr$D7F.$>xrt$D@L0$?,xv$DIR2$??0%$D[^6$?H6'$Ddd8$?"
           "Q<)$Dmj:$?ZB+$Dvp<$?cH-$E*v>$?lN/$E4'@$?uT1$E=-B$@)Z3$EF3D$@2`5$EO9"
           "F$@;f7$EX?H$@Dl9$EaEJ$@Mr;$EjKL$@Vx=$EsQN$@`)?$F'WP$@i/A$F0]R$@r5C$"
           "F9cT$A&;E$FBiV$A/AG$FKoX$A8GI$FTuZ$AAMK$F^&\\$AJSM$Fg,^$ASYO$Fp2`$A"
           "\\_Q$G$8b$AeeS$G->d$AnkU$G6Df$AwqW$G?Jh$B+wY$GHPj$B5([$GQVl$B>.]$GZ"
           "\\n$BG4_$Gcbp$BP:a$Glhr$BY@c$Gunt$BbFe$H)tv$BkLg$H3%x$BtRi$H<,%$C(X"
           "k$HE2'$C1^m$HN8)$C:do$HW>+$CCjq$H`D-$CLps$HiJ/$CUvu$HrP1$C_'w$I&V3$"
           "Ch.$$I/\\5$Cq4&$I8b7$D%:($IAh9$D.@*$IJn;$D7F,$ISt=$D@L.$I]%?$DIR0$I"
           "f+A$DRX2$Io1C$Ddd6$J,=G$Dmj8$J5CI$Dvp:$J>IK$E*v<$JGOM$E4'>$JPUO$E=-"
           "@$JY[Q$EF3B$JbaS$EO9D$JkgU$EX?F$JtmW$EaEH$K(sY$EjKJ$K2$[$EsQL$K;*]$"
           "F'WN$KD0_$F0]P$KM6a$F9cR$KV<c$FBiT$K_Be$FKoV$KhHg$FTuX$KqNi$F^&Z$L%"
           "Tk$Fg,\\$L.Zm$Fp2^$L7`o$G$8`$L@fq$G->b$LIls$G6Dd$LRru$G?Jf$L[xw$GHP"
           "h$Le*$$GQVj$Ln0&$GZ\\l$Lw6($Gcbn$M+<*$Glhp$M4B,$Gunr$M=H.$H)tt$MFN0"
           "$H3%v$MOT2$H<+x$MXZ4$HE2%$Ma`6$HN8'$Mjf8$HW>)$Msl:$H`D+$N'r<$HiJ-$N"
           "0x>$HrP/$N:)@$I&V1$NC/B$I/\\3$NL5D$I8b5$NU;F$IAh7$N^AH$IJn9$NgGJ$IS"
           "t;$NpML$I]%=$O$SN$If+?$O-YP$Io1A$O6_R$Ix7C$O?eT$J5CG$OQqX$J>II$OZwZ"
           "$JGOK$Od(\\$JPUM$Om.^$JY[O$Ov4`$JbaQ$P*:b$JkgS$P3@d$JtmU$P<Ff$K(sW$"
           "PELh$K2$Y$PNRj$K;*[$PWXl$KD0]$P`^n$KM6_$Pidp$KV<a$Prjr$K_Bc$Q&pt$Kh"
           "He$Q/vv$KqNg$Q9'x$L%Ti$QB.%$L.Zk$QK4'$L7`m$QT:)$L@fo$Q]@+$LIlq$QfF-"
           "$LRrs$QoL/$L[xu$QxR1$Le)w$R,X3$Ln0$$R5^5$Lw6&$R>d7$M+<($RGj9$M4B*$R"
           "Pp;$M=H,$RYv=$MFN.$Rc'?$MOT0$Rl-A$MXZ2$Ru3C$Ma`4$S)9E$Mjf6$S2?G$Msl"
           "8$S;EI$N'r:$SDKK$N0x<$SMQM$N:)>$SVWO$NC/@$S_]Q$NL5B$ShcS$NU;D$SqiU$"
           "N^AF$T%oW$NgGH$T.uY$NpMJ$T8&[$O$SL$TA,]$O-YN$TJ2_$O6_P$TS8a$O?eR$T"
           "\\>c$OHkT$TeDe$OZwX$TwPi$Od(Z$U+Vk$Om.\\$U4\\m$Ov4^$U=bo$P*:`$UFhq"
           "$P3@b$UOns$P<Fd$UXtu$PELf$Ub%w$PNRh$Uk,$$PWXj$Ut2&$P`^l$V(8($Pidn$"
           "V1>*$Prjp$V:D,$Q&pr$VCJ.$Q/vt$VLP0$Q9'v$VUV2$QB-x$V^\\4$QK4%$Vgb6$"
           "QT:'$Vph8$Q]@)$W$n:$QfF+$W-t<$QoL-$W7%>$QxR/$W@+@$R,X1$WI1B$R5^3$W"
           "R7D$R>d5$W[=F$RGj7$WdCH$RPp9$WmIJ$RYv;$WvOL$Rc'=$X*UN$Rl-?$X3[P$Ru"
           "3A$X<aR$S)9C$XEgT$S2?E$XNmV$S;EG$XWsX$SDKI$Xa$Z$SMQK$Xj*\\$SVWM$Xs"
           "0^$S_]O$Y'6`$ShcQ$Y0<b$SqiS$Y9Bd$T%oU$YBHf$T.uW$YKNh$T8&Y$YTTj$TA,"
           "[$Y]Zl$TJ2]$Yf`n$TS8_$Yofp$T\\>a$Yxlr$TeDc$Z,rt$TnJe$Z5xv$U+Vi$ZH0"
           "%$U4\\k$ZQ6'$U=bm$ZZ<)$UFho$ZcB+$UOnq$ZlH-$UXts$ZuN/$Ub%u$[)T1$Uk+"
           "w$[2Z3$Ut2$$[;`5$V(8&$[Df7$V1>($[Ml9$V:D*$[Vr;$VCJ,$[_x=$VLP.$[i)?"
           "$VUV0$[r/A$V^\\2$\\&5C$Vgb4$\\/;E$Vph6$\\8AG$W$n8$\\AGI$W-t:$\\JMK"
           "$W7%<$\\SSM$W@+>$\\\\YO$WI1@$\\e_Q$WR7B$\\neS$W[=D$\\wkU$WdCF$]+qW"
           "$WmIH$]4wY$WvOJ$]>([$X*UL$]G.]$X3[N$]P4_$X<aP$]Y:a$XEgR$]b@c$XNmT$"
           "]kFe$XWsV$]tLg$Xa$X$^(Ri$Xj*Z$^1Xk$Xs0\\$^:^m$Y'6^$^Cdo$Y0<`$^Ljq$"
           "Y9Bb$^Ups$YBHd$^^vu$YKNf$^h'w$YTTh$^q.$$Y]Zj$_%4&$Yf`l$_.:($Yofn$_"
           "7@*$Yxlp$_@F,$Z,rr$_IL.$Z5xt$_RR0$Z?)v$_[X2$ZQ6%$_md6$ZZ<'$_vj8$Zc"
           "B)$`*p:$ZlH+$`3v<$ZuN-$`='>$[)T/$`F-@$[2Z1$`O3B$[;`3$`X9D$[Df5$`a?"
           "F$[Ml7$`jEH$[Vr9$`sKJ$[_x;$a'QL$[i)=$a0WN$[r/?$a9]P$\\&5A$aBcR$\\/"
           ";C$aKiT$\\8AE$aToV$\\AGG$a]uX$\\JMI$ag&Z$\\SSK$ap,\\$\\\\YM$b$2^$"
           "\\e_O$b-8`$\\neQ$b6>b$\\wkS$b?Dd$]+qU$bHJf$]4wW$bQPh$]>(Y$bZVj$]G"
           ".[$bc\\l$]P4]$blbn$]Y:_$buhp$]b@a$c)nr$]kFc$c2tt$]tLe$c<%v$^(Rg$cE"
           "+x$^1Xi$cN2%$^:^k$cW8'$^Cdm$c`>)$^Ljo$ciD+$^Upq$crJ-$^^vs$d&P/$^h'"
           "u$d/V1$^q-w$d8\\3$_%4$$dAb5$_.:&$dJh7$_7@($dSn9$_@F*$d\\t;$_IL,$df"
           "%=$_RR.$do+?$_[X0$dx1A$_d^2$e,7C$_vj6$e>CG$`*p8$eGII$`3v:$ePOK$`='"
           "<$eYUM$`F->$eb[O$`O3@$ekaQ$`X9B$etgS$`a?D$f(mU$`jEF$f1sW$`sKH$f;$Y$"
           "a'QJ$fD*[$a0WL$fM0]$a9]N$fV6_$aBcP$f_<a$aKiR$fhBc$aToT$fqHe$a]uV$g%"
           "Ng$ag&X$g.Ti$ap,Z$g7Zk$b$2\\$g@`m$b-8^$gIfo$b6>`$gRlq$b?Db$g[rs$bHJ"
           "d$gdxu$bQPf$gn)w$bZVh$gw0$$bc\\j$h+6&$blbl$h4<($buhn$h=B*$c)np$hFH,"
           "$c2tr$hON.$c<%t$hXT0$cE+v$haZ2$cN1x$hj`4$cW8%$hsf6$c`>'$i'l8$ciD)$i"
           "0r:$crJ+$i9x<$d&P-$iC)>$d/V/$iL/@$d8\\1$iU5B$dAb3$i^;D$dJh5$igAF$dS"
           "n7$ipGH$d\\t9$j$MJ$df%;$j-SL$do+=$j6YN$dx1?$j?_P$e,7A$jHeR$e5=C$jQk"
           "T$eGIG$jcwX$ePOI$jm(Z$eYUK$jv.\\$eb[M$k*4^$ekaO$k3:`$etgQ$k<@b$f(mS"
           "$kEFd$f1sU$kNLf$f;$W$kWRh$fD*Y$k`Xj$fM0[$ki^l$fV6]$krdn$f_<_$l&jp$f"
           "hBa$l/pr$fqHc$l8vt$g%Ne$lB'v$g.Tg$lK-x$g7Zi$lT4%$g@`k$l]:'$gIfm$lf@"
           ")$gRlo$loF+$g[rq$lxL-$gdxs$m,R/$gn)u$m5X1$gw/w$m>^3$h+6$$mGd5$h4<&$"
           "mPj7$h=B($mYp9$hFH*$mbv;$hON,$ml'=$hXT.$mu-?$haZ0$n)3A$hj`2$n29C$hs"
           "f4$n;?E$i'l6$nDEG$i0r8$nMKI$i9x:$nVQK$iC)<$n_WM$iL/>$nh]O$iU5@$nqcQ"
           "$i^;B$o%iS$igAD$o.oU$ipGF$o7uW$j$MH$oA&Y$j-SJ$oJ,[$j6YL$oS2]$j?_N$o"
           "\\8_$jHeP$oe>a$jQkR$onDc$jZqT$owJe$jm(X$p4Vi$jv.Z$p=\\k$k*4\\$pFbm$"
           "k3:^$pOho$k<@`$pXnq$kEFb$pats$kNLd$pk%u$kWRf$pt+w$k`Xh$q(2$$ki^j$q1"
           "8&$krdl$q:>($l&jn$qCD*$l/pp$qLJ,$l8vr$qUP.$lB't$q^V0$lK-v$qg\\2$lT3"
           "x$qpb4$l]:%$r$h6$lf@'$r-n8$loF)$r6t:$lxL+$r@%<$m,R-$rI+>$m5X/$rR1@$"
           "m>^1$r[7B$mGd3$rd=D$mPj5$rmCF$mYp7$rvIH$mbv9$s*OJ$ml';$s3UL$mu-=$s<"
           "[N$n)3?$sEaP$n29A$sNgR$n;?C$sWmT$nDEE$s`sV$nMKG$sj$X$nVQI$ss*Z$n_WK"
           "$t'0\\$nh]M$t06^$nqcO$t9<`$o%iQ$tBBb$o.oS$tKHd$o7uU$tTNf$oA&W$t]Th$"
           "oJ,Y$tfZj$oS2[$to`l$o\\8]$txfn$oe>_$u,lp$onDa$u5rr$owJc$u>xt$p+Pe$u"
           "H)v$p=\\i$uZ6%$pFbk$uc<'$pOhm$ulB)$pXno$uuH+$patq$v)N-$pk%s$v2T/$pt"
           "+u$v;Z1$q(1w$vD`3$q18$$vMf5$q:>&$vVl7$qCD($v_r9$qLJ*$vhx;$qUP,$vr)="
           "$q^V.$w&/?$qg\\0$w/5A$qpb2$w8;C$r$h4$wAAE$r-n6$wJGG$r6t8$wSMI$r@%:$"
           "w\\SK$rI+<$weYM$rR1>$wn_O$r[7@$wweQ$rd=B$x+kS$rmCD$x4qU$rvIF$x=wW$s"
           "*OH$xG(Y$s3UJ$xP.[$s<[L$xY4]$sEaN$xb:_$sNgP$xk@a$sWmR$xtFc$s`sT%$(L"
           "e$sj$V%$1Rg$ss*X%$:Xi$t'0Z%$C^k$t06\\%$Ldm$t9<^%$Ujo$tBB`%$^pq$tKHb"
           "%$gvs$tTNd%$q'u$t]Tf%%%-w$tfZh%%.4$$to`j%%7:&$txfl%%@@($u,ln%%IF*$u"
           "5rp%%RL,$u>xr%%[R.$uH)t%%dX0$uQ/v%%m^2$uc<%%&*j6$ulB'%&3p8$uuH)%&<v"
           ":$v)N+%&F'<$v2T-%&O->$v;Z/%&X3@$vD`1%&a9B$vMf3%&j?D$vVl5%&sEF$v_r7%"
           "''KH$vhx9%'0QJ$vr);%'9WL$w&/=%'B]N$w/5?%'KcP$w8;A%'TiR$wAAC%']oT$wJ"
           "GE%'fuV$wSMG%'p&X$w\\SI%($,Z$weYK%(-2\\$wn_M%(68^$wweO%(?>`$x+kQ%(H"
           "Db$x4qS%(QJd$x=wU%(ZPf$xG(W%(cVh$xP.Y%(l\\j$xY4[%(ubl$xb:]%))hn$xk@"
           "_%)2np$xtFa%);tr%$(Lc%)E%t%$1Re%)N+v%$:Xg%)W1x%$C^i%)`8%%$Ldk%)i>'%"
           "$Ujm%)rD)%$^po%*&J+%$gvq%*/P-%$q's%*8V/%%%-u%*A\\1%%.3w%*Jb3%%7:$%*"
           "Sh5%%@@&%*\\n7%%IF(%*et9%%RL*%*o%;%%[R,%*x+=%%dX.%+,1?%%m^0%+57A%%v"
           "d2%+>=C%&3p6%+PIG%&<v8%+YOI%&F':%+bUK%&O-<%+k[M%&X3>%+taO%&a9@%,(gQ"
           "%&j?B%,1mS%&sED%,:sU%''KF%,D$W%'0QH%,M*Y%'9WJ%,V0[%'B]L%,_6]%'KcN%,"
           "h<_%'TiP%,qBa%']oR%-%Hc%'fuT%-.Ne%'p&V%-7Tg%($,X%-@Zi%(-2Z%-I`k%(68"
           "\\%-Rfm%(?>^%-[lo%(HD`%-drq%(QJb%-mxs%(ZPd%-w)u%(cVf%.+/w%(l\\h%.46"
           "$%(ubj%.=<&%))hl%.FB(%)2nn%.OH*%);tp%.XN,%)E%r%.aT.%)N+t%.jZ0%)W1v%"
           ".s`2%)`7x%/'f4%)i>%%/0l6%)rD'%/9r8%*&J)%/Bx:%*/P+%/L)<%*8V-%/U/>%*A"
           "\\/%/^5@%*Jb1%/g;B%*Sh3%/pAD%*\\n5%0$GF%*et7%0-MH%*o%9%06SJ%*x+;%0?"
           "YL%+,1=%0H_N%+57?%0QeP%+>=A%0ZkR%+GCC%0cqT%+YOG%0v(X%+bUI%1*.Z%+k[K"
           "%134\\%+taM%1<:^%,(gO%1E@`%,1mQ%1NFb%,:sS%1WLd%,D$U%1`Rf%,M*W%1iXh%"
           ",V0Y%1r^j%,_6[%2&dl%,h<]%2/jn%,qB_%28pp%-%Ha%2Avr%-.Nc%2K't%-7Te%2T"
           "-v%-@Zg%2]3x%-I`i%2f:%%-Rfk%2o@'%-[lm%2xF)%-dro%3,L+%-mxq%35R-%-w)s"
           "%3>X/%.+/u%3G^1%.45w%3Pd3%.=<$%3Yj5%.FB&%3bp7%.OH(%3kv9%.XN*%3u';%."
           "aT,%4)-=%.jZ.%423?%.s`0%4;9A%/'f2%4D?C%/0l4%4MEE%/9r6%4VKG%/Bx8%4_Q"
           "I%/L):%4hWK%/U/<%4q]M%/^5>%5%cO%/g;@%5.iQ%/pAB%57oS%0$GD%5@uU%0-MF%"
           "5J&W%06SH%5S,Y%0?YJ%5\\2[%0H_L%5e8]%0QeN%5n>_%0ZkP%5wDa%0cqR%6+Jc%0"
           "lwT%64Pe%1*.X%6F\\i%134Z%6Obk%1<:\\%6Xhm%1E@^%6ano%1NF`%6jtq%1WLb%6"
           "t%s%1`Rd%7(+u%1iXf%711w%1r^h%7:8$%2&dj%7C>&%2/jl%7LD(%28pn%7UJ*%2Av"
           "p%7^P,%2K'r%7gV.%2T-t%7p\\0%2]3v%8$b2%2f9x%8-h4%2o@%%86n6%2xF'%8?t8"
           "%3,L)%8I%:%35R+%8R+<%3>X-%8[1>%3G^/%8d7@%3Pd1%8m=B%3Yj3%8vCD%3bp5%9"
           "*IF%3kv7%93OH%3u'9%9<UJ%4)-;%9E[L%423=%9NaN%4;9?%9WgP%4D?A%9`mR%4MEC"
           "%9isT%4VKE%9s$V%4_QG%:'*X%4hWI%:00Z%4q]K%:96\\%5%cM%:B<^%5.iO%:KB`%5"
           "7oQ%:THb%5@uS%:]Nd%5J&U%:fTf%5S,W%:oZh%5\\2Y%:x`j%5e8[%;,fl%5n>]%;5l"
           "n%5wD_%;>rp%6+Ja%;Gxr%64Pc%;Q)t%6=Ve%;Z/v%6Obi%;l<%%6Xhk%;uB'%6anm%<"
           ")H)%6jto%<2N+%6t%q%<;T-%7(+s%<DZ/%711u%<M`1%7:7w%<Vf3%7C>$%<_l5%7LD&"
           "%<hr7%7UJ(%<qx9%7^P*%=&);%7gV,%=//=%7p\\.%=85?%8$b0%=A;A%8-h2%=JAC%8"
           "6n4%=SGE%8?t6%=\\MG%8I%8%=eSI%8R+:%=nYK%8[1<%=w_M%8d7>%>+eO%8m=@%>4k"
           "Q%8vCB%>=qS%9*ID%>FwU%93OF%>P(W%9<UH%>Y.Y%9E[J%>b4[%9NaL%>k:]%9WgN%>"
           "t@_%9`mP%?(Fa%9isR%?1Lc%9s$T%?:Re%:'*V%?CXg%:00X%?L^i%:96Z%?Udk%:B<"
           "\\%?^jm%:KB^%?gpo%:TH`%?pvq%:]Nb%@%'s%:fTd%@.-u%:oZf%@73w%:x`h%@@:$%"
           ";,fj%@I@&%;5ll%@RF(%;>rn%@[L*%;Gxp%@dR,%;Q)r%@mX.%;Z/t%@v^0%;c5v%A*d"
           "2%;uB%%A<p6%<)H'%AEv8%<2N)%AO':%<;T+%AX-<%<DZ-%Aa3>%<M`/%Aj9@%<Vf1%A"
           "s?B%<_l3%B'ED%<hr5%B0KF%<qx7%B9QH%=&)9%BBWJ%=//;%BK]L%=85=%BTcN%=A;?"
           "%B]iP%=JAA%BfoR%=SGC%BouT%=\\ME%C$&V%=eSG%C-,X%=nYI%C62Z%=w_K%C?8\\%"
           ">+eM%CH>^%>4kO%CQD`%>=qQ%CZJb%>FwS%CcPd%>P(U%ClVf%>Y.W%Cu\\h%>b4Y%D)"
           "bj%>k:[%D2hl%>t@]%D;nn%?(F_%DDtp%?1La%DN%r%?:Rc%DW+t%?CXe%D`1v%?L^g%"
           "Di7x%?Udi%Dr>%%?^jk%E&D'%?gpm%E/J)%?pvo%E8P+%@%'q%EAV-%@.-s%EJ\\/%@7"
           "3u%ESb1%@@9w%E\\h3%@I@$%Een5%@RF&%Ent7%@[L(%Ex%9%@dR*%F,+;%@mX,%F51="
           "%@v^.%F>7?%A*d0%FG=A%A3j2%FPCC%AEv6%FbOG%AO'8%FkUI%AX-:%Ft[K%Aa3<%G("
           "aM%Aj9>%G1gO%As?@%G:mQ%B'EB%GCsS%B0KD%GM$U%B9QF%GV*W%BBWH%G_0Y%BK]J%"
           "Gh6[%BTcL%Gq<]%B]iN%H%B_%BfoP%H.Ha%BouR%H7Nc%C$&T%H@Te%C-,V%HIZg%C62"
           "X%HR`i%C?8Z%H[fk%CH>\\%Hdlm%CQD^%Hmro%CZJ`%Hvxq%CcPb%I+)s%ClVd%I4/u%"
           "Cu\\f%I=5w%D)bh%IF<$%D2hj%IOB&%D;nl%IXH(%DDtn%IaN*%DN%p%IjT,%DW+r%Is"
           "Z.%D`1t%J'`0%Di7v%J0f2%Dr=x%J9l4%E&D%%JBr6%E/J'%JKx8%E8P)%JU):%EAV+%"
           "J^/<%EJ\\-%Jg5>%ESb/%Jp;@%E\\h1%K$AB%Een3%K-GD%Ent5%K6MF%Ex%7%K?SH%F"
           ",+9%KHYJ%F51;%KQ_L%F>7=%KZeN%FG=?%KckP%FPCA%KlqR%FYIC%KuwT%FkUG%L3.X"
           "%Ft[I%L<4Z%G(aK%LE:\\%G1gM%LN@^%G:mO%LWF`%GCsQ%L`Lb%GM$S%LiRd%GV*U%L"
           "rXf%G_0W%M&^h%Gh6Y%M/dj%Gq<[%M8jl%H%B]%MApn%H.H_%MJvp%H7Na%MT'r%H@Tc"
           "%M]-t%HIZe%Mf3v%HR`g%Mo9x%H[fi%Mx@%%Hdlk%N,F'%Hmrm%N5L)%Hvxo%N>R+%I+"
           ")q%NGX-%I4/s%NP^/%I=5u%NYd1%IF;w%Nbj3%IOB$%Nkp5%IXH&%Ntv7%IaN(%O)'9%"
           "IjT*%O2-;%IsZ,%O;3=%J'`.%OD9?%J0f0%OM?A%J9l2%OVEC%JBr4%O_KE%JKx6%OhQ"
           "G%JU)8%OqWI%J^/:%P%]K%Jg5<%P.cM%Jp;>%P7iO%K$A@%P@oQ%K-GB%PIuS%K6MD%P"
           "S&U%K?SF%P\\,W%KHYH%Pe2Y%KQ_J%Pn8[%KZeL%Pw>]%KckN%Q+D_%KlqP%Q4Ja%Kuw"
           "R%Q=Pc%L*(T%QFVe%L<4X%QXbi%LE:Z%Qahk%LN@\\%Qjnm%LWF^%Qsto%L`L`%R(%q%"
           "LiRb%R1+s%LrXd%R:1u%M&^f%RC7w%M/dh%RL>$%M8jj%RUD&%MApl%R^J(%MJvn%RgP"
           "*%MT'p%RpV,%M]-r%S$\\.%Mf3t%S-b0%Mo9v%S6h2%Mx?x%S?n4%N,F%%SHt6%N5L'%"
           "SR%8%N>R)%S[+:%NGX+%Sd1<%NP^-%Sm7>%NYd/%Sv=@%Nbj1%T*CB%Nkp3%T3ID%Ntv"
           "5%T<OF%O)'7%TEUH%O2-9%TN[J%O;3;%TWaL%OD9=%T`gN%OM??%TimP%OVEA%TrsR%O"
           "_KC%U'$T%OhQE%U0*V%OqWG%U90X%P%]I%UB6Z%P.cK%UK<\\%P7iM%UTB^%P@oO%U]H"
           "`%PIuQ%UfNb%PS&S%UoTd%P\\,U%UxZf%Pe2W%V,`h%Pn8Y%V5fj%Pw>[%V>ll%Q+D]%"
           "VGrn%Q4J_%VPxp%Q=Pa%VZ)r%QFVc%Vc/t%QO\\e%Vl5v%Qahi%W)B%%Qjnk%W2H'%Qs"
           "tm%W;N)%R(%o%WDT+%R1+q%WMZ-%R:1s%WV`/%RC7u%W_f1%RL=w%Whl3%RUD$%Wqr5%"
           "R^J&%X%x7%RgP(%X/)9%RpV*%X8/;%S$\\,%XA5=%S-b.%XJ;?%S6h0%XSAA%S?n2%X"
           "\\GC%SHt4%XeME%SR%6%XnSG%S[+8%XwYI%Sd1:%Y+_K%Sm7<%Y4eM%Sv=>%Y=kO%T*C"
           "@%YFqQ%T3IB%YOwS%T<OD%YY(U%TEUF%Yb.W%TN[H%Yk4Y%TWaJ%Yt:[%T`gL%Z(@]%T"
           "imN%Z1F_%TrsP%Z:La%U'$R%ZCRc%U0*T%ZLXe%U90V%ZU^g%UB6X%Z^di%UK<Z%Zgjk"
           "%UTB\\%Zppm%U]H^%[$vo%UfN`%[.'q%UoTb%[7-s%UxZd%[@3u%V,`f%[I9w%V5fh%["
           "R@$%V>lj%[[F&%VGrl%[dL(%VPxn%[mR*%VZ)p%[vX,%Vc/r%\\*^.%Vl5t%\\3d0%Vu"
           ";v%\\<j2%W2H%%\\Nv6%W;N'%\\X'8%WDT)%\\a-:%WMZ+%\\j3<%WV`-%\\s9>%W_f/"
           "%]'?@%Whl1%]0EB%Wqr3%]9KD%X%x5%]BQF%X/)7%]KWH%X8/9%]T]J%XA5;%]]cL%XJ"
           ";=%]fiN%XSA?%]ooP%X\\GA%]xuR%XeMC%^-&T%XnSE%^6,V%XwYG%^?2X%Y+_I%^H8Z"
           "%Y4eK%^Q>\\%Y=kM%^ZD^%YFqO%^cJ`%YOwQ%^lPb%YY(S%^uVd%Yb.U%_)\\f%Yk4W%"
           "_2bh%Yt:Y%_;hj%Z(@[%_Dnl%Z1F]%_Mtn%Z:L_%_W%p%ZCRa%_`+r%ZLXc%_i1t%ZU^"
           "e%_r7v%Z^dg%`&=x%Zgji%`/D%%Zppk%`8J'%[$vm%`AP)%[.'o%`JV+%[7-q%`S\\-%"
           "[@3s%`\\b/%[I9u%`eh1%[R?w%`nn3%[[F$%`wt5%[dL&%a,%7%[mR(%a5+9%[vX*%a>"
           "1;%\\*^,%aG7=%\\3d.%aP=?%\\<j0%aYCA%\\Ep2%abIC%\\X'6%atUG%\\a-8%b([I"
           "%\\j3:%b1aK%\\s9<%b:gM%]'?>%bCmO%]0E@%bLsQ%]9KB%bV$S%]BQD%b_*U%]KWF%"
           "bh0W%]T]H%bq6Y%]]cJ%c%<[%]fiL%c.B]%]ooN%c7H_%]xuP%c@Na%^-&R%cITc%^6,"
           "T%cRZe%^?2V%c[`g%^H8X%cdfi%^Q>Z%cmlk%^ZD\\%cvrm%^cJ^%d*xo%^lP`%d4)q%"
           "^uVb%d=/s%_)\\d%dF5u%_2bf%dO;w%_;hh%dXB$%_Dnj%daH&%_Mtl%djN(%_W%n%ds"
           "T*%_`+p%e'Z,%_i1r%e0`.%_r7t%e9f0%`&=v%eBl2%`/Cx%eKr4%`8J%%eTx6%`AP'%"
           "e^)8%`JV)%eg/:%`S\\+%ep5<%`\\b-%f$;>%`eh/%f-A@%`nn1%f6GB%`wt3%f?MD%a"
           ",%5%fHSF%a5+7%fQYH%a>19%fZ_J%aG7;%fceL%aP==%flkN%aYC?%fuqP%abIA%g)wR"
           "%akOC%g3(T%b([G%gE4X%b1aI%gN:Z%b:gK%gW@\\%bCmM%g`F^%bLsO%giL`%bV$Q%g"
           "rRb%b_*S%h&Xd%bh0U%h/^f%bq6W%h8dh%c%<Y%hAjj%c.B[%hJpl%c7H]%hSvn%c@N_"
           "%h]'p%cITa%hf-r%cRZc%ho3t%c[`e%hx9v%cdfg%i,?x%cmli%i5F%%cvrk%i>L'%d*"
           "xm%iGR)%d4)o%iPX+%d=/q%iY^-%dF5s%ibd/%dO;u%ikj1%dXAw%itp3%daH$%j(v5%"
           "djN&%j2'7%dsT(%j;-9%e'Z*%jD3;%e0`,%jM9=%e9f.%jV??%eBl0%j_EA%eKr2%jhK"
           "C%eTx4%jqQE%e^)6%k%WG%eg/8%k.]I%ep5:%k7cK%f$;<%k@iM%f-A>%kIoO%f6G@%k"
           "RuQ%f?MB%k\\&S%fHSD%ke,U%fQYF%kn2W%fZ_H%kw8Y%fceJ%l+>[%flkL%l4D]%fuq"
           "N%l=J_%g)wP%lFPa%g3(R%lOVc%g<.T%lX\\e%gN:X%ljhi%gW@Z%lsnk%g`F\\%m'tm"
           "%giL^%m1%o%grR`%m:+q%h&Xb%mC1s%h/^d%mL7u%h8df%mU=w%hAjh%m^D$%hJpj%mg"
           "J&%hSvl%mpP(%h]'n%n$V*%hf-p%n-\\,%ho3r%n6b.%hx9t%n?h0%i,?v%nHn2%i5Ex"
           "%nQt4%i>L%%n[%6%iGR'%nd+8%iPX)%nm1:%iY^+%nv7<%ibd-%o*=>%ikj/%o3C@%it"
           "p1%o<IB%j(v3%oEOD%j2'5%oNUF%j;-7%oW[H%jD39%o`aJ%jM9;%oigL%jV?=%ormN%"
           "j_E?%p&sP%jhKA%p0$R%jqQC%p9*T%k%WE%pB0V%k.]G%pK6X%k7cI%pT<Z%k@iK%p]B"
           "\\%kIoM%pfH^%kRuO%poN`%k\\&Q%pxTb%ke,S%q,Zd%kn2U%q5`f%kw8W%q>fh%l+>Y"
           "%qGlj%l4D[%qPrl%l=J]%qYxn%lFP_%qc)p%lOVa%ql/r%lX\\c%qu5t%labe%r);v%l"
           "sni%r;H%%m'tk%rDN'%m1%m%rMT)%m:+o%rVZ+%mC1q%r_`-%mL7s%rhf/%mU=u%rql1"
           "%m^Cw%s%r3%mgJ$%s.x5%mpP&%s8)7%n$V(%sA/9%n-\\*%sJ5;%n6b,%sS;=%n?h.%s"
           "\\A?%nHn0%seGA%nQt2%snMC%n[%4%swSE%nd+6%t+YG%nm18%t4_I%nv7:%t=eK%o*="
           "<%tFkM%o3C>%tOqO%o<I@%tXwQ%oEOB%tb(S%oNUD%tk.U%oW[F%tt4W%o`aH%u(:Y%o"
           "igJ%u1@[%ormL%u:F]%p&sN%uCL_%p0$P%uLRa%p9*R%uUXc%pB0T%u^^e%pK6V%ugdg"
           "%pT<X%upji%p]BZ%v$pk%pfH\\%v-vm%poN^%v7'o%pxT`%v@-q%q,Zb%vI3s%q5`d%v"
           "R9u%q>ff%v[?w%qGlh%vdF$%qPrj%vmL&%qYxl%vvR(%qc)n%w*X*%ql/p%w3^,%qu5r"
           "%w<d.%r);t%wEj0%r2Av%wNp2%rDN%%wa'6%rMT'%wj-8%rVZ)%ws3:%r_`+%x'9<%rh"
           "f-%x0?>%rql/%x9E@%s%r1%xBKB%s.x3%xKQD%s8)5%xTWF%sA/7%x]]H%sJ59%xfcJ%"
           "sS;;%xoiL%s\\A=%xxoN%seG?&$,uP%snMA&$6&R%swSC&$?,T%t+YE&$H2V%t4_G&$Q"
           "8X%t=eI&$Z>Z%tFkK&$cD\\%tOqM&$lJ^%tXwO&$uP`%tb(Q&%)Vb%tk.S&%2\\d%tt4"
           "U&%;bf%u(:W&%Dhh%u1@Y&%Mnj%u:F[&%Vtl%uCL]&%`%n%uLR_&%i+p%uUXa&%r1r%u"
           "^^c&&&7t%ugde&&/=v%upjg&&8Cx%v$pi&&AJ%%v-vk&&JP'%v7'm&&SV)%v@-o&&\\"
           "\\+%vI3q&&eb-%vR9s&&nh/%v[?u&&wn1%vdEw&'+t3%vmL$&'5%5%vvR&&'>+7%w*X("
           "&'G19%w3^*&'P7;%w<d,&'Y==%wEj.&'bC?%wNp0&'kIA%wWv2&'tOC%wj-6&(1[G%ws"
           "38&(:aI%x'9:&(CgK%x0?<&(LmM%x9E>&(UsO%xBK@&(_$Q%xKQB&(h*S%xTWD&(q0U%"
           "x]]F&)%6W%xfcH&).<Y%xoiJ&)7B[%xxoL&)@H]&$,uN&)IN_&$6&P&)RTa&$?,R&)[Z"
           "c&$H2T&)d`e&$Q8V&)mfg&$Z>X&)vli&$cDZ&**rk&$lJ\\&*3xm&$uP^&*=)o&%)V`&"
           "*F/q&%2\\b&*O5s&%;bd&*X;u&%Dhf&*aAw&%Mnh&*jH$&%Vtj&*sN&&%`%l&+'T(&%i"
           "+n&+0Z*&%r1p&+9`,&&&7r&+Bf.&&/=t&+Kl0&&8Cv&+Tr2&&AIx&+]x4&&JP%&+g)6&"
           "&SV'&+p/8&&\\\\)&,$5:&&eb+&,-;<&&nh-&,6A>&&wn/&,?G@&'+t1&,HMB&'5%3&,"
           "QSD&'>+5&,ZYF&'G17&,c_H&'P79&,leJ&'Y=;&,ukL&'bC=&-)qN&'kI?&-2wP&'tOA"
           "&-<(R&((UC&-E.T&(:aG&-W:X&(CgI&-`@Z&(LmK&-iF\\&(UsM&-rL^&(_$O&.&R`&("
           "h*Q&./Xb&(q0S&.8^d&)%6U&.Adf&).<W&.Jjh&)7BY&.Spj&)@H[&.\\vl&)IN]&.f'"
           "n&)RT_&.o-p&)[Za&.x3r&)d`c&/,9t&)mfe&/5?v&)vlg&/>Ex&**ri&/GL%&*3xk&/"
           "PR'&*=)m&/YX)&*F/o&/b^+&*O5q&/kd-&*X;s&/tj/&*aAu&0(p1&*jGw&01v3&*sN$"
           "&0;'5&+'T&&0D-7&+0Z(&0M39&+9`*&0V9;&+Bf,&0_?=&+Kl.&0hE?&+Tr0&0qKA&+]"
           "x2&1%QC&+g)4&1.WE&+p/6&17]G&,$58&1@cI&,-;:&1IiK&,6A<&1RoM&,?G>&1[uO&"
           ",HM@&1e&Q&,QSB&1n,S&,ZYD&1w2U&,c_F&2+8W&,leH&24>Y&,ukJ&2=D[&-)qL&2FJ"
           "]&-2wN&2OP_&-<(P&2XVa&-E.R&2a\\c&-N4T&2jbe&-`@X&3'ni&-iFZ&30tk&-rL\\"
           "&3:%m&.&R^&3C+o&./X`&3L1q&.8^b&3U7s&.Add&3^=u&.Jjf&3gCw&.Sph&3pJ$&."
           "\\vj&4$P&&.f'l&4-V(&.o-n&46\\*&.x3p&4?b,&/,9r&4Hh.&/5?t&4Qn0&/>Ev&4Z"
           "t2&/GKx&4d%4&/PR%&4m+6&/YX'&4v18&/b^)&5*7:&/kd+&53=<&/tj-&5<C>&0(p/&"
           "5EI@&01v1&5NOB&0;'3&5WUD&0D-5&5`[F&0M37&5iaH&0V99&5rgJ&0_?;&6&mL&0hE"
           "=&6/sN&0qK?&69$P&1%QA&6B*R&1.WC&6K0T&17]E&6T6V&1@cG&6]<X&1IiI&6fBZ&1"
           "RoK&6oH\\&1[uM&6xN^&1e&O&7,T`&1n,Q&75Zb&1w2S&7>`d&2+8U&7Gff&24>W&7Pl"
           "h&2=DY&7Yrj&2FJ[&7bxl&2OP]&7l)n&2XV_&7u/p&2a\\a&8)5r&2jbc&82;t&2she&"
           "8;Av&30ti&8MN%&3:%k&8VT'&3C+m&8_Z)&3L1o&8h`+&3U7q&8qf-&3^=s&9%l/&3gC"
           "u&9.r1&3pIw&97x3&4$P$&9A)5&4-V&&9J/7&46\\(&9S59&4?b*&9\\;;&4Hh,&9eA="
           "&4Qn.&9nG?&4Zt0&9wMA&4d%2&:+SC&4m+4&:4YE&4v16&:=_G&5*78&:FeI&53=:&:O"
           "kK&5<C<&:XqM&5EI>&:awO&5NO@&:k(Q&5WUB&:t.S&5`[D&;(4U&5iaF&;1:W&5rgH&"
           ";:@Y&6&mJ&;CF[&6/sL&;LL]&69$N&;UR_&6B*P&;^Xa&6K0R&;g^c&6T6T&;pde&6]<"
           "V&<$jg&6fBX&<-pi&6oHZ&<6vk&6xN\\&<@'m&7,T^&<I-o&75Z`&<R3q&7>`b&<[9s&"
           "7Gfd&<d?u&7Plf&<mEw&7Yrh&<vL$&7bxj&=*R&&7l)l&=3X(&7u/n&=<^*&8)5p&=Ed"
           ",&82;r&=Nj.&8;At&=Wp0&8DGv&=`v2&8VT%&=s-6&8_Z'&>'38&8h`)&>09:&8qf+&>"
           "9?<&9%l-&>BE>&9.r/&>KK@&97x1&>TQB&9A)3&>]WD&9J/5&>f]F&9S57&>ocH&9\\;"
           "9&>xiJ&9eA;&?,oL&9nG=&?5uN&9wM?&??&P&:+SA&?H,R&:4YC&?Q2T&:=_E&?Z8V&:"
           "FeG&?c>X&:OkI&?lDZ&:XqK&?uJ\\&:awM&@)P^&:k(O&@2V`&:t.Q&@;\\b&;(4S&@D"
           "bd&;1:U&@Mhf&;:@W&@Vnh&;CFY&@_tj&;LL[&@i%l&;UR]&@r+n&;^X_&A&1p&;g^a&"
           "A/7r&;pdc&A8=t&<$je&AACv&<-pg&AJIx&<6vi&ASP%&<@'k&A\\V'&<I-m&Ae\\)&<"
           "R3o&Anb+&<[9q&Awh-&<d?s&B+n/&<mEu&B4t1&<vKw&B>%3&=*R$&BG+5&=3X&&BP17"
           "&=<^(&BY79&=Ed*&Bb=;&=Nj,&BkC=&=Wp.&BtI?&=`v0&C(OA&=j'2&C1UC&>'36&CC"
           "aG&>098&CLgI&>9?:&CUmK&>BE<&C^sM&>KK>&Ch$O&>TQ@&Cq*Q&>]WB&D%0S&>f]D&"
           "D.6U&>ocF&D7<W&>xiH&D@BY&?,oJ&DIH[&?5uL&DRN]&??&N&D[T_&?H,P&DdZa&?Q2"
           "R&Dm`c&?Z8T&Dvfe&?c>V&E*lg&?lDX&E3ri&?uJZ&E<xk&@)P\\&EF)m&@2V^&EO/o&"
           "@;\\`&EX5q&@Dbb&Ea;s&@Mhd&EjAu&@Vnf&EsGw&@_th&F'N$&@i%j&F0T&&@r+l&F9"
           "Z(&A&1n&FB`*&A/7p&FKf,&A8=r&FTl.&AACt&F]r0&AJIv&Ffx2&ASOx&Fp)4&A\\V%"
           "&G$/6&Ae\\'&G-58&Anb)&G6;:&Awh+&G?A<&B+n-&GHG>&B4t/&GQM@&B>%1&GZSB&B"
           "G+3&GcYD&BP15&Gl_F&BY77&GueH&Bb=9&H)kJ&BkC;&H2qL&BtI=&H;wN&C(O?&HE(P"
           "&C1UA&HN.R&C:[C&HW4T&CLgG&Hi@X&CUmI&HrFZ&C^sK&I&L\\&Ch$M&I/R^&Cq*O&I"
           "8X`&D%0Q&IA^b&D.6S&IJdd&D7<U&ISjf&D@BW&I\\ph&DIHY&Ievj&DRN[&Io'l&D[T"
           "]&Ix-n&DdZ_&J,3p&Dm`a&J59r&Dvfc&J>?t&E*le&JGEv&E3rg&JPKx&E<xi&JYR%&E"
           "F)k&JbX'&EO/m&Jk^)&EX5o&Jtd+&Ea;q&K(j-&EjAs&K1p/&EsGu&K:v1&F'Mw&KD'3"
           "&F0T$&KM-5&F9Z&&KV37&FB`(&K_99&FKf*&Kh?;&FTl,&KqE=&F]r.&L%K?&Ffx0&L."
           "QA&Fp)2&L7WC&G$/4&L@]E&G-56&LIcG&G6;8&LRiI&G?A:&L[oK&GHG<&LduM&GQM>&"
           "Ln&O&GZS@&Lw,Q&GcYB&M+2S&Gl_D&M48U&GueF&M=>W&H)kH&MFDY&H2qJ&MOJ[&H;w"
           "L&MXP]&HE(N&MaV_&HN.P&Mj\\a&HW4R&Msbc&H`:T&N'he&HrFX&N9ti&I&LZ&NC%k&"
           "I/R\\&NL+m&I8X^&NU1o&IA^`&N^7q&IJdb&Ng=s&ISjd&NpCu&I\\pf&O$Iw&Ievh&O"
           "-P$&Io'j&O6V&&Ix-l&O?\\(&J,3n&OHb*&J59p&OQh,&J>?r&OZn.&JGEt&Oct0&JPK"
           "v&Om%2&JYQx&Ov+4&JbX%&P*16&Jk^'&P378&Jtd)&P<=:&K(j+&PEC<&K1p-&PNI>&K"
           ":v/&PWO@&KD'1&P`UB&KM-3&Pi[D&KV35&PraF&K_97&Q&gH&Kh?9&Q/mJ&KqE;&Q8sL"
           "&L%K=&QB$N&L.Q?&QK*P&L7WA&QT0R&L@]C&Q]6T&LIcE&Qf<V&LRiG&QoBX&L[oI&Qx"
           "HZ&LduK&R,N\\&Ln&M&R5T^&Lw,O&R>Z`&M+2Q&RG`b&M48S&RPfd&M=>U&RYlf&MFDW"
           "&Rbrh&MOJY&Rkxj&MXP[&Ru)l&MaV]&S)/n&Mj\\_&S25p&Msba&S;;r&N'hc&SDAt&N"
           "0ne&SMGv&NC%i&S_T%&NL+k&ShZ'&NU1m&Sq`)&N^7o&T%f+&Ng=q&T.l-&NpCs&T7r/"
           "&O$Iu&T@x1&O-Ow&TJ)3&O6V$&TS/5&O?\\&&T\\57&OHb(&Te;9&OQh*&TnA;&OZn,&"
           "TwG=&Oct.&U+M?&Om%0&U4SA&Ov+2&U=YC&P*14&UF_E&P376&UOeG&P<=8&UXkI&PEC"
           ":&UaqK&PNI<&UjwM&PWO>&Ut(O&P`U@&V(.Q&Pi[B&V14S&PraD&V::U&Q&gF&VC@W&Q"
           "/mH&VLFY&Q8sJ&VUL[&QB$L&V^R]&QK*N&VgX_&QT0P&Vp^a&Q]6R&W$dc&Qf<T&W-je"
           "&QoBV&W6pg&QxHX&W?vi&R,NZ&WI'k&R5T\\&WR-m&R>Z^&W[3o&RG``&Wd9q&RPfb&W"
           "m?s&RYld&WvEu&Rbrf&X*Kw&Rkxh&X3R$&Ru)j&X<X&&S)/l&XE^(&S25n&XNd*&S;;p"
           "&XWj,&SDAr&X`p.&SMGt&Xiv0&SVMv&Xs'2&ShZ%&Y036&Sq`'&Y998&T%f)&YB?:&T."
           "l+&YKE<&T7r-&YTK>&T@x/&Y]Q@&TJ)1&YfWB&TS/3&Yo]D&T\\55&YxcF&Te;7&Z,iH"
           "&TnA9&Z5oJ&TwG;&Z>uL&U+M=&ZH&N&U4S?&ZQ,P&U=YA&ZZ2R&UF_C&Zc8T&UOeE&Zl"
           ">V&UXkG&ZuDX&UaqI&[)JZ&UjwK&[2P\\&Ut(M&[;V^&V(.O&[D\\`&V14Q&[Mbb&V::"
           "S&[Vhd&VC@U&[_nf&VLFW&[hth&VULY&[r%j&V^R[&\\&+l&VgX]&\\/1n&Vp^_&\\87"
           "p&W$da&\\A=r&W-jc&\\JCt&W6pe&\\SIv&W?vg&\\\\Ox&WI'i&\\eV%&WR-k&\\n\\"
           "'&W[3m&\\wb)&Wd9o&]+h+&Wm?q&]4n-&WvEs&]=t/&X*Ku&]G%1&X3Qw&]P+3&X<X$&"
           "]Y15&XE^&&]b77&XNd(&]k=9&XWj*&]tC;&X`p,&^(I=&Xiv.&^1O?&Xs'0&^:UA&Y'-"
           "2&^C[C&Y996&^UgG&YB?8&^^mI&YKE:&^gsK&YTK<&^q$M&Y]Q>&_%*O&YfW@&_.0Q&Y"
           "o]B&_76S&YxcD&_@<U&Z,iF&_IBW&Z5oH&_RHY&Z>uJ&_[N[&ZH&L&_dT]&ZQ,N&_mZ_"
           "&ZZ2P&_v`a&Zc8R&`*fc&Zl>T&`3le&ZuDV&`<rg&[)JX&`Exi&[2PZ&`O)k&[;V\\&`"
           "X/m&[D\\^&`a5o&[Mb`&`j;q&[Vhb&`sAs&[_nd&a'Gu&[htf&a0Mw&[r%h&a9T$&\\&"
           "+j&aBZ&&\\/1l&aK`(&\\87n&aTf*&\\A=p&a]l,&\\JCr&afr.&\\SIt&aox0&\\\\O"
           "v&b$)2&\\eUx&b-/4&\\n\\%&b656&\\wb'&b?;8&]+h)&bHA:&]4n+&bQG<&]=t-&bZ"
           "M>&]G%/&bcS@&]P+1&blYB&]Y13&bu_D&]b75&c)eF&]k=7&c2kH&]tC9&c;qJ&^(I;&"
           "cDwL&^1O=&cN(N&^:U?&cW.P&^C[A&c`4R&^LaC&ci:T&^^mG&d&FX&^gsI&d/LZ&^q$"
           "K&d8R\\&_%*M&dAX^&_.0O&dJ^`&_76Q&dSdb&_@<S&d\\jd&_IBU&depf&_RHW&dnvh"
           "&_[NY&dx'j&_dT[&e,-l&_mZ]&e53n&_v`_&e>9p&`*fa&eG?r&`3lc&ePEt&`<re&eY"
           "Kv&`Exg&ebQx&`O)i&ekX%&`X/k&et^'&`a5m&f(d)&`j;o&f1j+&`sAq&f:p-&a'Gs&"
           "fCv/&a0Mu&fM'1&a9Sw&fV-3&aBZ$&f_35&aK`&&fh97&aTf(&fq?9&a]l*&g%E;&afr"
           ",&g.K=&aox.&g7Q?&b$)0&g@WA&b-/2&gI]C&b654&gRcE&b?;6&g[iG&bHA8&gdoI&b"
           "QG:&gmuK&bZM<&gw&M&bcS>&h+,O&blY@&h42Q&bu_B&h=8S&c)eD&hF>U&c2kF&hODW"
           "&c;qH&hXJY&cDwJ&haP[&cN(L&hjV]&cW.N&hs\\_&c`4P&i'ba&ci:R&i0hc&cr@T&i"
           "9ne&d/LX&iL%i&d8RZ&iU+k&dAX\\&i^1m&dJ^^&ig7o&dSd`&ip=q&d\\jb&j$Cs&de"
           "pd&j-Iu&dnvf&j6Ow&dx'h&j?V$&e,-j&jH\\&&e53l&jQb(&e>9n&jZh*&eG?p&jcn,"
           "&ePEr&jlt.&eYKt&jv%0&ebQv&k*+2&ekWx&k314&et^%&k<76&f(d'&kE=8&f1j)&kN"
           "C:&f:p+&kWI<&fCv-&k`O>&fM'/&kiU@&fV-1&kr[B&f_33&l&aD&fh95&l/gF&fq?7&"
           "l8mH&g%E9&lAsJ&g.K;&lK$L&g7Q=&lT*N&g@W?&l]0P&gI]A&lf6R&gRcC&lo<T&g[i"
           "E&lxBV&gdoG&m,HX&gmuI&m5NZ&gw&K&m>T\\&h+,M&mGZ^&h42O&mP``&h=8Q&mYfb&"
           "hF>S&mbld&hODU&mkrf&hXJW&mtxh&haPY&n))j&hjV[&n2/l&hs\\]&n;5n&i'b_&nD"
           ";p&i0ha&nMAr&i9nc&nVGt&iBte&n_Mv&iU+i&nqZ%&i^1k&o%`'&ig7m&o.f)&ip=o&"
           "o7l+&j$Cq&o@r-&j-Is&oIx/&j6Ou&oS)1&j?Uw&o\\/3&jH\\$&oe55&jQb&&on;7&j"
           "Zh(&owA9&jcn*&p+G;&jlt,&p4M=&jv%.&p=S?&k*+0&pFYA&k312&pO_C&k<74&pXeE"
           "&kE=6&pakG&kNC8&pjqI&kWI:&pswK&k`O<&q((M&kiU>&q1.O&kr[@&q:4Q&l&aB&qC"
           ":S&l/gD&qL@U&l8mF&qUFW&lAsH&q^LY&lK$J&qgR[&lT*L&qpX]&l]0N&r$^_&lf6P&"
           "r-da&lo<R&r6jc&lxBT&r?pe&m,HV&rHvg&m5NX&rR'i&m>TZ&r[-k&mGZ\\&rd3m&mP"
           "`^&rm9o&mYf`&rv?q&mblb&s*Es&mkrd&s3Ku&mtxf&s<Qw&n))h&sEX$&n2/j&sN^&&"
           "n;5l&sWd(&nD;n&s`j*&nMAp&sip,&nVGr&srv.&n_Mt&t''0&nhSv&t0-2&o%`%&tB9"
           "6&o.f'&tK?8&o7l)&tTE:&o@r+&t]K<&oIx-&tfQ>&oS)/&toW@&o\\/1&tx]B&oe53&"
           "u,cD&on;5&u5iF&owA7&u>oH&p+G9&uGuJ&p4M;&uQ&L&p=S=&uZ,N&pFY?&uc2P&pO_"
           "A&ul8R&pXeC&uu>T&pakE&v)DV&pjqG&v2JX&pswI&v;PZ&q((K&vDV\\&q1.M&vM\\^"
           "&q:4O&vVb`&qC:Q&v_hb&qL@S&vhnd&qUFU&vqtf&q^LW&w&%h&qgRY&w/+j&qpX[&w8"
           "1l&r$^]&wA7n&r-d_&wJ=p&r6ja&wSCr&r?pc&w\\It&rHve&weOv&rR'g&wnUx&r[-i"
           "&ww\\%&rd3k&x+b'&rm9m&x4h)&rv?o&x=n+&s*Eq&xFt-&s3Ks&xP%/&s<Qu&xY+1&s"
           "EWw&xb13&sN^$&xk75&sWd&&xt=7&s`j('$(C9&sip*'$1I;&srv,'$:O=&t''.'$CU?"
           "&t0-0'$L[A&t932'$UaC&tK?6'$gmG&tTE8'$psI&t]K:'%%$K&tfQ<'%.*M&toW>'%7"
           "0O&tx]@'%@6Q&u,cB'%I<S&u5iD'%RBU&u>oF'%[HW&uGuH'%dNY&uQ&J'%mT[&uZ,L'"
           "%vZ]&uc2N'&*`_&ul8P'&3fa&uu>R'&<lc&v)DT'&Ere&v2JV'&Nxg&v;PX'&X)i&vDV"
           "Z'&a/k&vM\\\\'&j5m&vVb^'&s;o&v_h`'''Aq&vhnb''0Gs&vqtd''9Mu&w&%f''BSw"
           "&w/+h''KZ$&w81j''T`&&wA7l'']f(&wJ=n''fl*&wSCp''or,&w\\Ir''xx.&weOt'("
           "-)0&wnUv'(6/2&ww[x'(?54&x+b%'(H;6&x4h''(QA8&x=n)'(ZG:&xFt+'(cM<&xP%-"
           "'(lS>&xY+/'(uY@&xb11'))_B&xk73')2eD&xt=5');kF'$(C7')DqH'$1I9')MwJ'$:"
           "O;')W(L'$CU=')`.N'$L[?')i4P'$UaA')r:R'$^gC'*&@T'$psG'*8LX'%%$I'*ARZ'"
           "%.*K'*JX\\'%70M'*S^^'%@6O'*\\d`'%I<Q'*ejb'%RBS'*npd'%[HU'*wvf'%dNW'+"
           ",'h'%mTY'+5-j'%vZ['+>3l'&*`]'+G9n'&3f_'+P?p'&<la'+YEr'&Erc'+bKt'&Nxe"
           "'+kQv'&X)g'+tWx'&a/i',(^%'&j5k',1d''&s;m',:j)'''Ao',Cp+''0Gq',Lv-''9"
           "Ms',V'/''BSu',_-1''KYw',h33''T`$',q95'']f&'-%?7''fl('-.E9''or*'-7K;'"
           "'xx,'-@Q='(-).'-IW?'(6/0'-R]A'(?52'-[cC'(H;4'-diE'(QA6'-moG'(ZG8'-vu"
           "I'(cM:'.+&K'(lS<'.4,M'(uY>'.=2O'))_@'.F8Q')2eB'.O>S');kD'.XDU')DqF'."
           "aJW')MwH'.jPY')W(J'.sV[')`.L'/'\\]')i4N'/0b_')r:P'/9ha'*&@R'/Bnc'*/F"
           "T'/Kte'*ARX'/^+i'*JXZ'/g1k'*S^\\'/p7m'*\\d^'0$=o'*ej`'0-Cq'*npb'06Is"
           "'*wvd'0?Ou'+,'f'0HUw'+5-h'0Q\\$'+>3j'0Zb&'+G9l'0ch('+P?n'0ln*'+YEp'0"
           "ut,'+bKr'1*%.'+kQt'13+0'+tWv'1<12',(]x'1E74',1d%'1N=6',:j''1WC8',Cp)"
           "'1`I:',Lv+'1iO<',V'-'1rU>',_-/'2&[@',h31'2/aB',q93'28gD'-%?5'2AmF'-."
           "E7'2JsH'-7K9'2T$J'-@Q;'2]*L'-IW='2f0N'-R]?'2o6P'-[cA'2x<R'-diC'3,BT'"
           "-moE'35HV'-vuG'3>NX'.+&I'3GTZ'.4,K'3PZ\\'.=2M'3Y`^'.F8O'3bf`'.O>Q'3k"
           "lb'.XDS'3trd'.aJU'4(xf'.jPW'42)h'.sVY'4;/j'/'\\['4D5l'/0b]'4M;n'/9h_"
           "'4VAp'/Bna'4_Gr'/Ktc'4hMt'/U%e'4qSv'/g1i'5.`%'/p7k'57f''0$=m'5@l)'0-"
           "Co'5Ir+'06Iq'5Rx-'0?Os'5\\)/'0HUu'5e/1'0Q[w'5n53'0Zb$'5w;5'0ch&'6+A7"
           "'0ln('64G9'0ut*'6=M;'1*%,'6FS='13+.'6OY?'1<10'6X_A'1E72'6aeC'1N=4'6j"
           "kE'1WC6'6sqG'1`I8'7'wI'1iO:'71(K'1rU<'7:.M'2&[>'7C4O'2/a@'7L:Q'28gB'"
           "7U@S'2AmD'7^FU'2JsF'7gLW'2T$H'7pRY'2]*J'8$X['2f0L'8-^]'2o6N'86d_'2x<"
           "P'8?ja'3,BR'8Hpc'35HT'8Qve'3>NV'8['g'3GTX'8d-i'3PZZ'8m3k'3Y`\\'8v9m'"
           "3bf^'9*?o'3kl`'93Eq'3trb'9<Ks'4(xd'9EQu'42)f'9NWw'4;/h'9W^$'4D5j'9`d"
           "&'4M;l'9ij('4VAn'9rp*'4_Gp':&v,'4hMr':0'.'4qSt':9-0'5%Yv':B32";
  // Should start with: 0, 1, 52, 51, 1, 2, 53, 52, 2, 3, 54, ...
  if (gcore::Base85::Decode(dec, decstr, outdata, outlen)) {
    unsigned int *vals = (unsigned int*)outdata;
    unsigned int count = outlen / 4;
    std::cout << "Decoded " << count << " indices" << std::endl;
    std::cout << "  [0] " << vals[0] << std::endl;
    std::cout << "  [1] " << vals[1] << std::endl;
    std::cout << "  [2] " << vals[2] << std::endl;
    std::cout << "  [3] " << vals[3] << std::endl;
    std::cout << "  ..." << std::endl;
    std::cout << "  [" << (count / 2) << "] " << vals[count/2] << std::endl;
    std::cout << "  ..." << std::endl;
    std::cout << "  [" << (count - 6) << "] " << vals[count-6] << std::endl;
    std::cout << "  [" << (count - 5) << "] " << vals[count-5] << std::endl;
    std::cout << "  [" << (count - 4) << "] " << vals[count-4] << std::endl;
    std::cout << "  [" << (count - 3) << "] " << vals[count-3] << std::endl;
    std::cout << "  [" << (count - 2) << "] " << vals[count-2] << std::endl;
    std::cout << "  [" << (count - 1) << "] " << vals[count-1] << std::endl;
    
    es = gcore::Base85::Encode(enc, outdata, outlen);
    std::cout << "=> Re-encoded to: " << es << std::endl;
    if (es != decstr) {
      std::cout << "!!! DIFFERS FROM ORIGINAL !!!" << std::endl;
    }
    
    free(outdata);
  }
  
  gcore::Base85::DestroyDecoder(dec);
  gcore::Base85::DestroyEncoder(enc);
  
  enc = gcore::Base85::CreateEncoder("pack1");
  dec = gcore::Base85::CreateDecoder("pack1");
  outdata = 0;
  outlen = 0;
  decstr = "aDq99Y&XMt89+]caAMwnY&XMt89+]ca>*aNY&XMt89+]ca:\\K.Y&XM"
           "t89+]ca794cY&XMt89+]ca0G\\xY&XMt89+]ca)V08Y&XMt89+]c`ps"
           "+bY&XMt89+]czY&XMt89+]c7e-P7Y&XMt89+]c7reTbY&XMt89+]c8$"
           "W,MY&XMt89+]c8+HY8Y&XMt89+]c8.koXY&XMt89+]c82:0xY&XMt89"
           "+]c85]GCY&XMt89+]c89+]cY&XMt89+]caDq99Xx57T85]GCaAMwnXx"
           "57T85]GCa>*aNXx57T85]GCa:\\K.Xx57T85]GCa794cXx57T85]GCa"
           "0G\\xXx57T85]GCa)V08Xx57T85]GC`ps+bXx57T85]GCzXx57T85]G"
           "C7e-P7Xx57T85]GC7reTbXx57T85]GC8$W,MXx57T85]GC8+HY8Xx57"
           "T85]GC8.koXXx57T85]GC82:0xXx57T85]GC85]GCXx57T85]GC89+]"
           "cXx57T85]GCaDq99Xtfv482:0xaAMwnXtfv482:0xa>*aNXtfv482:0"
           "xa:\\K.Xtfv482:0xa794cXtfv482:0xa0G\\xXtfv482:0xa)V08Xt"
           "fv482:0x`ps+bXtfv482:0xzXtfv482:0x7e-P7Xtfv482:0x7reTbX"
           "tfv482:0x8$W,MXtfv482:0x8+HY8Xtfv482:0x8.koXXtfv482:0x8"
           "2:0xXtfv482:0x85]GCXtfv482:0x89+]cXtfv482:0xaDq99XqC_i8"
           ".koXaAMwnXqC_i8.koXa>*aNXqC_i8.koXa:\\K.XqC_i8.koXa794c"
           "XqC_i8.koXa0G\\xXqC_i8.koXa)V08XqC_i8.koX`ps+bXqC_i8.ko"
           "XzXqC_i8.koX7e-P7XqC_i8.koX7reTbXqC_i8.koX8$W,MXqC_i8.k"
           "oX8+HY8XqC_i8.koX8.koXXqC_i8.koX82:0xXqC_i8.koX85]GCXqC"
           "_i8.koX89+]cXqC_i8.koXaDq99XmuII8+HY8aAMwnXmuII8+HY8a>*"
           "aNXmuII8+HY8a:\\K.XmuII8+HY8a794cXmuII8+HY8a0G\\xXmuII8"
           "+HY8a)V08XmuII8+HY8`ps+bXmuII8+HY8zXmuII8+HY87e-P7XmuII"
           "8+HY87reTbXmuII8+HY88$W,MXmuII8+HY88+HY8XmuII8+HY88.koX"
           "XmuII8+HY882:0xXmuII8+HY885]GCXmuII8+HY889+]cXmuII8+HY8"
           "aDq99Xg.q^8$W,MaAMwnXg.q^8$W,Ma>*aNXg.q^8$W,Ma:\\K.Xg.q"
           "^8$W,Ma794cXg.q^8$W,Ma0G\\xXg.q^8$W,Ma)V08Xg.q^8$W,M`ps"
           "+bXg.q^8$W,MzXg.q^8$W,M7e-P7Xg.q^8$W,M7reTbXg.q^8$W,M8$"
           "W,MXg.q^8$W,M8+HY8Xg.q^8$W,M8.koXXg.q^8$W,M82:0xXg.q^8$"
           "W,M85]GCXg.q^8$W,M89+]cXg.q^8$W,MaDq99X`=Ds7reTbaAMwnX`"
           "=Ds7reTba>*aNX`=Ds7reTba:\\K.X`=Ds7reTba794cX`=Ds7reTba"
           "0G\\xX`=Ds7reTba)V08X`=Ds7reTb`ps+bX`=Ds7reTbzX`=Ds7reT"
           "b7e-P7X`=Ds7reTb7reTbX`=Ds7reTb8$W,MX`=Ds7reTb8+HY8X`=D"
           "s7reTb8.koXX`=Ds7reTb82:0xX`=Ds7reTb85]GCX`=Ds7reTb89+]"
           "cX`=Ds7reTbaDq99XRZ@H7e-P7aAMwnXRZ@H7e-P7a>*aNXRZ@H7e-P"
           "7a:\\K.XRZ@H7e-P7a794cXRZ@H7e-P7a0G\\xXRZ@H7e-P7a)V08XR"
           "Z@H7e-P7`ps+bXRZ@H7e-P7zXRZ@H7e-P77e-P7XRZ@H7e-P77reTbX"
           "RZ@H7e-P78$W,MXRZ@H7e-P78+HY8XRZ@H7e-P78.koXXRZ@H7e-P78"
           "2:0xXRZ@H7e-P785]GCXRZ@H7e-P789+]cXRZ@H7e-P7aDq99zzaAMw"
           "nzza>*aNzza:\\K.zza794czza0G\\xzza)V08zz`ps+b!$$$$$$$$$"
           ")7e-P7zz7reTbzz8$W,Mzz8+HY8zz8.koXzz82:0xzz85]GCzz89+]c"
           "zzaDq99/Fidr`ps+baAMwn/Fidr`ps+ba>*aN/Fidr`ps+ba:\\K./F"
           "idr`ps+ba794c/Fidr`ps+ba0G\\x/Fidr`ps+ba)V08/Fidr`ps+b`"
           "ps+b/Fidr`ps+bz/Fidr`ps+b7e-P7/Fidr`ps+b7reTb/Fidr`ps+b"
           "8$W,M/Fidr`ps+b8+HY8/Fidr`ps+b8.koX/Fidr`ps+b82:0x/Fidr"
           "`ps+b85]GC/Fidr`ps+b89+]c/Fidr`ps+baDq99/TLiHa)V08aAMwn"
           "/TLiHa)V08a>*aN/TLiHa)V08a:\\K./TLiHa)V08a794c/TLiHa)V0"
           "8a0G\\x/TLiHa)V08a)V08/TLiHa)V08`ps+b/TLiHa)V08z/TLiHa)"
           "V087e-P7/TLiHa)V087reTb/TLiHa)V088$W,M/TLiHa)V088+HY8/T"
           "LiHa)V088.koX/TLiHa)V0882:0x/TLiHa)V0885]GC/TLiHa)V0889"
           "+]c/TLiHa)V08aDq99/[>A3a0G\\xaAMwn/[>A3a0G\\xa>*aN/[>A3"
           "a0G\\xa:\\K./[>A3a0G\\xa794c/[>A3a0G\\xa0G\\x/[>A3a0G\\"
           "xa)V08/[>A3a0G\\x`ps+b/[>A3a0G\\xz/[>A3a0G\\x7e-P7/[>A3"
           "a0G\\x7reTb/[>A3a0G\\x8$W,M/[>A3a0G\\x8+HY8/[>A3a0G\\x8"
           ".koX/[>A3a0G\\x82:0x/[>A3a0G\\x85]GC/[>A3a0G\\x89+]c/[>"
           "A3a0G\\xaDq99/b/msa794caAMwn/b/msa794ca>*aN/b/msa794ca:"
           "\\K./b/msa794ca794c/b/msa794ca0G\\x/b/msa794ca)V08/b/ms"
           "a794c`ps+b/b/msa794cz/b/msa794c7e-P7/b/msa794c7reTb/b/m"
           "sa794c8$W,M/b/msa794c8+HY8/b/msa794c8.koX/b/msa794c82:0"
           "x/b/msa794c85]GC/b/msa794c89+]c/b/msa794caDq99/eS/>a:\\"
           "K.aAMwn/eS/>a:\\K.a>*aN/eS/>a:\\K.a:\\K./eS/>a:\\K.a794"
           "c/eS/>a:\\K.a0G\\x/eS/>a:\\K.a)V08/eS/>a:\\K.`ps+b/eS/>"
           "a:\\K.z/eS/>a:\\K.7e-P7/eS/>a:\\K.7reTb/eS/>a:\\K.8$W,M"
           "/eS/>a:\\K.8+HY8/eS/>a:\\K.8.koX/eS/>a:\\K.82:0x/eS/>a:"
           "\\K.85]GC/eS/>a:\\K.89+]c/eS/>a:\\K.aDq99/hvE^a>*aNaAMw"
           "n/hvE^a>*aNa>*aN/hvE^a>*aNa:\\K./hvE^a>*aNa794c/hvE^a>*"
           "aNa0G\\x/hvE^a>*aNa)V08/hvE^a>*aN`ps+b/hvE^a>*aNz/hvE^a"
           ">*aN7e-P7/hvE^a>*aN7reTb/hvE^a>*aN8$W,M/hvE^a>*aN8+HY8/"
           "hvE^a>*aN8.koX/hvE^a>*aN82:0x/hvE^a>*aN85]GC/hvE^a>*aN8"
           "9+]c/hvE^a>*aNaDq99/lD\\)aAMwnaAMwn/lD\\)aAMwna>*aN/lD"
           "\\)aAMwna:\\K./lD\\)aAMwna794c/lD\\)aAMwna0G\\x/lD\\)aA"
           "Mwna)V08/lD\\)aAMwn`ps+b/lD\\)aAMwnz/lD\\)aAMwn7e-P7/lD"
           "\\)aAMwn7reTb/lD\\)aAMwn8$W,M/lD\\)aAMwn8+HY8/lD\\)aAMw"
           "n8.koX/lD\\)aAMwn82:0x/lD\\)aAMwn85]GC/lD\\)aAMwn89+]c/"
           "lD\\)aAMwnaDq99/ogrIaDq99aAMwn/ogrIaDq99a>*aN/ogrIaDq99"
           "a:\\K./ogrIaDq99a794c/ogrIaDq99a0G\\x/ogrIaDq99a)V08/og"
           "rIaDq99`ps+b/ogrIaDq99z/ogrIaDq997e-P7/ogrIaDq997reTb/o"
           "grIaDq998$W,M/ogrIaDq998+HY8/ogrIaDq998.koX/ogrIaDq9982"
           ":0x/ogrIaDq9985]GC/ogrIaDq9989+]c/ogrIaDq99";
  if (gcore::Base85::Decode(dec, decstr, outdata, outlen)) {
    float *vals = (float*) outdata;
    unsigned int count = (outlen / 4) / 3;
    unsigned int idx = 0;
    std::cout << "Decoded " << count << " points" << std::endl;
    std::cout << "  [0] " << vals[0] << ", " << vals[1] << ", " << vals[2] << std::endl;
    std::cout << "  [1] " << vals[3] << ", " << vals[4] << ", " << vals[5] << std::endl;
    std::cout << "  [2] " << vals[6] << ", " << vals[7] << ", " << vals[8] << std::endl;
    std::cout << "  ..." << std::endl;
    //std::cout << "  [" << (count / 2) << "] " << vals[3*count/2] << ", " << vals[1+3*count/2] << ", " << vals[2+3*count/2] << std::endl;
    idx = (count / 2) - 1;
    std::cout << "  [" << idx << "] " << vals[3*idx] << ", " << vals[1+3*idx] << ", " << vals[2+3*idx] << std::endl;
    idx = (count / 2);
    std::cout << "  [" << idx << "] " << vals[3*idx] << ", " << vals[1+3*idx] << ", " << vals[2+3*idx] << std::endl;
    idx = (count / 2) + 1;
    std::cout << "  [" << idx << "] " << vals[3*idx] << ", " << vals[1+3*idx] << ", " << vals[2+3*idx] << std::endl;
    std::cout << "  ..." << std::endl;
    std::cout << "  [" << (count - 3) << "] " << vals[3*(count-3)] << ", " << vals[1+3*(count-3)] << ", " << vals[2+3*(count-3)] << std::endl;
    std::cout << "  [" << (count - 2) << "] " << vals[3*(count-2)] << ", " << vals[1+3*(count-2)] << ", " << vals[2+3*(count-2)] << std::endl;
    std::cout << "  [" << (count - 1) << "] " << vals[3*(count-1)] << ", " << vals[1+3*(count-1)] << ", " << vals[2+3*(count-1)] << std::endl;
    
    es = gcore::Base85::Encode(enc, outdata, outlen);
    std::cout << "=> Re-encoded to: " << es << std::endl;
    if (es != decstr) {
      // it is actually true, but the data match
      // it has to do with RLE encoding:
      //   in the sample data 0 5 times is !$$$$$$$$$(
      //   while this encoder would encode !z$$$$$(, be because zzzzz is yet smaller
      //   the repeat pattern is ignored
      std::cout << "!!! DIFFERS FROM ORIGINAL !!!" << std::endl;
      std::cout << "length: " << decstr.length() << " / " << es.length() << std::endl;
      
      free(outdata);
      outdata = 0;
      outlen = 0;
      
      if (gcore::Base85::Decode(dec, es, outdata, outlen)) {
        float *vals = (float*) outdata;
        unsigned int count = (outlen / 4) / 3;
        std::cout << "Decoded " << count << " points" << std::endl;
        std::cout << "  [0] " << vals[0] << ", " << vals[1] << ", " << vals[2] << std::endl;
        std::cout << "  [1] " << vals[3] << ", " << vals[4] << ", " << vals[5] << std::endl;
        std::cout << "  [2] " << vals[6] << ", " << vals[7] << ", " << vals[8] << std::endl;
        std::cout << "  ..." << std::endl;
        idx = (count / 2) - 1;
        std::cout << "  [" << idx << "] " << vals[3*idx] << ", " << vals[1+3*idx] << ", " << vals[2+3*idx] << std::endl;
        idx = (count / 2);
        std::cout << "  [" << idx << "] " << vals[3*idx] << ", " << vals[1+3*idx] << ", " << vals[2+3*idx] << std::endl;
        idx = (count / 2) + 1;
        std::cout << "  [" << idx << "] " << vals[3*idx] << ", " << vals[1+3*idx] << ", " << vals[2+3*idx] << std::endl;
        std::cout << "  ..." << std::endl;
        std::cout << "  [" << (count - 3) << "] " << vals[3*(count-3)] << ", " << vals[1+3*(count-3)] << ", " << vals[2+3*(count-3)] << std::endl;
        std::cout << "  [" << (count - 2) << "] " << vals[3*(count-2)] << ", " << vals[1+3*(count-2)] << ", " << vals[2+3*(count-2)] << std::endl;
        std::cout << "  [" << (count - 1) << "] " << vals[3*(count-1)] << ", " << vals[1+3*(count-1)] << ", " << vals[2+3*(count-1)] << std::endl;
        
        free(outdata);
      }
      
    } else {
      free(outdata);
    }
  }
  
  gcore::Base85::DestroyEncoder(enc);
  gcore::Base85::DestroyDecoder(dec);
  
  return 0;
}
