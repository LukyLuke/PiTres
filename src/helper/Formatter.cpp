/*
	Functions to format different Strings into different standard formats
	Copyright (C) 2012  Lukas Zurschmiede <l.zurschmiede@delightsoftware.com>

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "Formatter.h"

#include <QString>
#include <QRegExp>
#include <QLocale>
#include <QDebug>

QString Formatter::telephone(QString number) {
	return Formatter::telephone(number, QLocale::Switzerland);
}

QString Formatter::telephone(QString number, QLocale::Country country) {
	QRegExp rx("^(\\+\\+?|00)?\\s*(\\d+\\((\\d+)\\))?([\\d\\s\\-\\.\\/\\']+)$");
	QString back("");
	
	if (number.isEmpty()) {
		return back;
	}
	
	if (rx.indexIn(number) >= 0) {
		// with international prefix
		if (rx.cap(1).length() > 0) {
			back.append(rx.cap(1).replace("+", "00").replace("++", "00"));
		} else {
			back.append("0041");
		}
		
		// with national prefix in brackets
		if (rx.cap(3).length() > 0) {
			if (rx.cap(3).at(0) == '0') {
				back.append(rx.cap(3).remove(0, 1));
			} else {
				back.append(rx.cap(3));
			}
		}
		
		// with international prefix
		if (rx.cap(4).length() > 0) {
			back.append(rx.cap(4).replace(QRegExp("[^\\d]"), ""));
		}
		
		return back;
	}
	qDebug() << "Invalid Telephone Number: " << number << " - Unknown Format...";
	return back;
}

QString Formatter::email(QString email) {
	//QRegExp rx("^$");
	return email;
}

QString Formatter::number(float number, int precision) {
	precision = abs(precision);
	QString back = QString::number(round(number* 10) / 10, 'f', precision);
	
	// Position of the thousand separator - if we have decimals, we must move it by that amount to left
	int pos = 3;
	if (precision > 0) {
		pos += precision + 1;
	}
	
	// Add a separator on each
	while (back.size() > pos) {
		back.insert(back.size() - pos, '\'');
		pos += 4;
	}
	return back;
}

QString Formatter::currency(float amount) {
	QLocale locale;
	QString back = Formatter::number(amount);
	return back.append(" ").append(locale.currencySymbol());
}

QLocale::Country Formatter::iso3166country(QString iso3166) {
	iso3166 = iso3166.toLower();
	if (iso3166 == "af") {
		return (QLocale::Afghanistan);
	} else if (iso3166 == "al") {
		return (QLocale::Albania);
	} else if (iso3166 == "dz") {
		return (QLocale::Algeria);
	} else if (iso3166 == "as") {
		return (QLocale::AmericanSamoa);
	} else if (iso3166 == "vi") {
		return (QLocale::Andorra);
	} else if (iso3166 == "ao") {
		return (QLocale::Angola);
	} else if (iso3166 == "ai") {
		return (QLocale::Anguilla);
	} else if (iso3166 == "aq") {
		return (QLocale::Antarctica);
	} else if (iso3166 == "ag") {
		return (QLocale::AntiguaAndBarbuda);
	} else if (iso3166 == "ar") {
		return (QLocale::Argentina);
	} else if (iso3166 == "am") {
		return (QLocale::Armenia);
	} else if (iso3166 == "aw") {
		return (QLocale::Aruba);
	} else if (iso3166 == "au") {
		return (QLocale::Australia);
	} else if (iso3166 == "at") {
		return (QLocale::Austria);
	} else if (iso3166 == "az") {
		return (QLocale::Azerbaijan);
	} else if (iso3166 == "bs") {
		return (QLocale::Bahamas);
	} else if (iso3166 == "bh") {
		return (QLocale::Bahrain);
	} else if (iso3166 == "bd") {
		return (QLocale::Bangladesh);
	} else if (iso3166 == "bb") {
		return (QLocale::Barbados);
	} else if (iso3166 == "by") {
		return (QLocale::Belarus);
	} else if (iso3166 == "be") {
		return (QLocale::Belgium);
	} else if (iso3166 == "bz") {
		return (QLocale::Belize);
	} else if (iso3166 == "bj") {
		return (QLocale::Benin);
	} else if (iso3166 == "bm") {
		return (QLocale::Bermuda);
	} else if (iso3166 == "bt") {
		return (QLocale::Bhutan);
	} else if (iso3166 == "bo") {
		return (QLocale::Bolivia);
	} else if (iso3166 == "ba") {
		return (QLocale::BosniaAndHerzegowina);
	} else if (iso3166 == "bw") {
		return (QLocale::Botswana);
	} else if (iso3166 == "bv") {
		return (QLocale::BouvetIsland);
	} else if (iso3166 == "br") {
		return (QLocale::Brazil);
	} else if (iso3166 == "io") {
		return (QLocale::BritishIndianOceanTerritory);
	} else if (iso3166 == "bn") {
		return (QLocale::BruneiDarussalam);
	} else if (iso3166 == "bg") {
		return (QLocale::Bulgaria);
	} else if (iso3166 == "bf") {
		return (QLocale::BurkinaFaso);
	} else if (iso3166 == "bi") {
		return (QLocale::Burundi);
	} else if (iso3166 == "kh") {
		return (QLocale::Cambodia);
	} else if (iso3166 == "cm") {
		return (QLocale::Cameroon);
	} else if (iso3166 == "ca") {
		return (QLocale::Canada);
	} else if (iso3166 == "cv") {
		return (QLocale::CapeVerde);
	} else if (iso3166 == "ky") {
		return (QLocale::CaymanIslands);
	} else if (iso3166 == "cf") {
		return (QLocale::CentralAfricanRepublic);
	} else if (iso3166 == "td") {
		return (QLocale::Chad);
	} else if (iso3166 == "cl") {
		return (QLocale::Chile);
	} else if (iso3166 == "cn") {
		return (QLocale::China);
	} else if (iso3166 == "cx") {
		return (QLocale::ChristmasIsland);
	} else if (iso3166 == "cc") {
		return (QLocale::CocosIslands);
	} else if (iso3166 == "co") {
		return (QLocale::Colombia);
	} else if (iso3166 == "km") {
		return (QLocale::Comoros);
	} else if (iso3166 == "cd") {
		return (QLocale::DemocraticRepublicOfCongo);
	} else if (iso3166 == "cg") {
		return (QLocale::PeoplesRepublicOfCongo);
	} else if (iso3166 == "ck") {
		return (QLocale::CookIslands);
	} else if (iso3166 == "cr") {
		return (QLocale::CostaRica);
	} else if (iso3166 == "ci") {
		return (QLocale::IvoryCoast);
	} else if (iso3166 == "hr") {
		return (QLocale::Croatia);
	} else if (iso3166 == "cu") {
		return (QLocale::Cuba);
	} else if (iso3166 == "cy") {
		return (QLocale::Cyprus);
	} else if (iso3166 == "cz") {
		return (QLocale::CzechRepublic);
	} else if (iso3166 == "dk") {
		return (QLocale::Denmark);
	} else if (iso3166 == "dj") {
		return (QLocale::Djibouti);
	} else if (iso3166 == "dm") {
		return (QLocale::Dominica);
	} else if (iso3166 == "do") {
		return (QLocale::DominicanRepublic);
	} else if (iso3166 == "tp") {
		return (QLocale::EastTimor);
	} else if (iso3166 == "ec") {
		return (QLocale::Ecuador);
	} else if (iso3166 == "eg") {
		return (QLocale::Egypt);
	} else if (iso3166 == "sv") {
		return (QLocale::ElSalvador);
	} else if (iso3166 == "gq") {
		return (QLocale::EquatorialGuinea);
	} else if (iso3166 == "er") {
		return (QLocale::Eritrea);
	} else if (iso3166 == "ee") {
		return (QLocale::Estonia);
	} else if (iso3166 == "et") {
		return (QLocale::Ethiopia);
	} else if (iso3166 == "fk") {
		return (QLocale::FalklandIslands);
	} else if (iso3166 == "fo") {
		return (QLocale::FaroeIslands);
	} else if (iso3166 == "fj") {
		return (QLocale::FijiCountry);
	} else if (iso3166 == "fi") {
		return (QLocale::Finland);
	} else if (iso3166 == "fr") {
		return (QLocale::France);
	} else if (iso3166 == "fx") {
		return (QLocale::MetropolitanFrance);
	} else if (iso3166 == "gf") {
		return (QLocale::FrenchGuiana);
	} else if (iso3166 == "pf") {
		return (QLocale::FrenchPolynesia);
	} else if (iso3166 == "tf") {
		return (QLocale::FrenchSouthernTerritories);
	} else if (iso3166 == "ga") {
		return (QLocale::Gabon);
	} else if (iso3166 == "gm") {
		return (QLocale::Gambia);
	} else if (iso3166 == "ge") {
		return (QLocale::Georgia);
	} else if (iso3166 == "de") {
		return (QLocale::Germany);
	} else if (iso3166 == "gh") {
		return (QLocale::Ghana);
	} else if (iso3166 == "gi") {
		return (QLocale::Gibraltar);
	} else if (iso3166 == "gr") {
		return (QLocale::Greece);
	} else if (iso3166 == "gl") {
		return (QLocale::Greenland);
	} else if (iso3166 == "gd") {
		return (QLocale::Grenada);
	} else if (iso3166 == "gp") {
		return (QLocale::Guadeloupe);
	} else if (iso3166 == "gu") {
		return (QLocale::Guam);
	} else if (iso3166 == "gt") {
		return (QLocale::Guatemala);
	} else if (iso3166 == "gn") {
		return (QLocale::Guinea);
	} else if (iso3166 == "gw") {
		return (QLocale::GuineaBissau);
	} else if (iso3166 == "gy") {
		return (QLocale::Guyana);
	} else if (iso3166 == "ht") {
		return (QLocale::Haiti);
	} else if (iso3166 == "hm") {
		return (QLocale::HeardAndMcDonaldIslands);
	} else if (iso3166 == "hn") {
		return (QLocale::Honduras);
	} else if (iso3166 == "hk") {
		return (QLocale::HongKong);
	} else if (iso3166 == "hu") {
		return (QLocale::Hungary);
	} else if (iso3166 == "is") {
		return (QLocale::Iceland);
	} else if (iso3166 == "in") {
		return (QLocale::India);
	} else if (iso3166 == "id") {
		return (QLocale::Indonesia);
	} else if (iso3166 == "ir") {
		return (QLocale::Iran);
	} else if (iso3166 == "iq") {
		return (QLocale::Iraq);
	} else if (iso3166 == "ie") {
		return (QLocale::Ireland);
	} else if (iso3166 == "il") {
		return (QLocale::Israel);
	} else if (iso3166 == "it") {
		return (QLocale::Italy);
	} else if (iso3166 == "jm") {
		return (QLocale::Jamaica);
	} else if (iso3166 == "jp") {
		return (QLocale::Japan);
	} else if (iso3166 == "jo") {
		return (QLocale::Jordan);
	} else if (iso3166 == "kz") {
		return (QLocale::Kazakhstan);
	} else if (iso3166 == "ke") {
		return (QLocale::Kenya);
	} else if (iso3166 == "ki") {
		return (QLocale::Kiribati);
	} else if (iso3166 == "kp") {
		return (QLocale::DemocraticRepublicOfKorea);
	} else if (iso3166 == "kr") {
		return (QLocale::RepublicOfKorea);
	} else if (iso3166 == "kw") {
		return (QLocale::Kuwait);
	} else if (iso3166 == "kg") {
		return (QLocale::Kyrgyzstan);
	} else if (iso3166 == "la") {
		return (QLocale::Lao);
	} else if (iso3166 == "lv") {
		return (QLocale::Latvia);
	} else if (iso3166 == "lb") {
		return (QLocale::Lebanon);
	} else if (iso3166 == "ls") {
		return (QLocale::Lesotho);
	} else if (iso3166 == "lr") {
		return (QLocale::Liberia);
	} else if (iso3166 == "ly") {
		return (QLocale::LibyanArabJamahiriya);
	} else if (iso3166 == "li") {
		return (QLocale::Liechtenstein);
	} else if (iso3166 == "lt") {
		return (QLocale::Lithuania);
	} else if (iso3166 == "lu") {
		return (QLocale::Luxembourg);
	} else if (iso3166 == "mo") {
		return (QLocale::Macau);
	} else if (iso3166 == "mk") {
		return (QLocale::Macedonia);
	} else if (iso3166 == "mg") {
		return (QLocale::Madagascar);
	} else if (iso3166 == "mw") {
		return (QLocale::Malawi);
	} else if (iso3166 == "my") {
		return (QLocale::Malaysia);
	} else if (iso3166 == "mv") {
		return (QLocale::Maldives);
	} else if (iso3166 == "ml") {
		return (QLocale::Mali);
	} else if (iso3166 == "mt") {
		return (QLocale::Malta);
	} else if (iso3166 == "mh") {
		return (QLocale::MarshallIslands);
	} else if (iso3166 == "mq") {
		return (QLocale::Martinique);
	} else if (iso3166 == "mr") {
		return (QLocale::Mauritania);
	} else if (iso3166 == "mu") {
		return (QLocale::Mauritius);
	} else if (iso3166 == "yt") {
		return (QLocale::Mayotte);
	} else if (iso3166 == "mx") {
		return (QLocale::Mexico);
	} else if (iso3166 == "fm") {
		return (QLocale::Micronesia);
	} else if (iso3166 == "md") {
		return (QLocale::Moldova);
	} else if (iso3166 == "mc") {
		return (QLocale::Monaco);
	} else if (iso3166 == "mn") {
		return (QLocale::Mongolia);
	} else if (iso3166 == "ms") {
		return (QLocale::Montserrat);
	} else if (iso3166 == "ma") {
		return (QLocale::Morocco);
	} else if (iso3166 == "mz") {
		return (QLocale::Mozambique);
	} else if (iso3166 == "mm") {
		return (QLocale::Myanmar);
	} else if (iso3166 == "na") {
		return (QLocale::Namibia);
	} else if (iso3166 == "nr") {
		return (QLocale::NauruCountry);
	} else if (iso3166 == "np") {
		return (QLocale::Nepal);
	} else if (iso3166 == "nl") {
		return (QLocale::Netherlands);
	} else if (iso3166 == "an") {
		return (QLocale::NetherlandsAntilles);
	} else if (iso3166 == "nc") {
		return (QLocale::NewCaledonia);
	} else if (iso3166 == "nz") {
		return (QLocale::NewZealand);
	} else if (iso3166 == "ni") {
		return (QLocale::Nicaragua);
	} else if (iso3166 == "ne") {
		return (QLocale::Niger);
	} else if (iso3166 == "ng") {
		return (QLocale::Nigeria);
	} else if (iso3166 == "nu") {
		return (QLocale::Niue);
	} else if (iso3166 == "nf") {
		return (QLocale::NorfolkIsland);
	} else if (iso3166 == "np") {
		return (QLocale::NorthernMarianaIslands);
	} else if (iso3166 == "no") {
		return (QLocale::Norway);
	} else if (iso3166 == "om") {
		return (QLocale::Oman);
	} else if (iso3166 == "pk") {
		return (QLocale::Pakistan);
	} else if (iso3166 == "pw") {
		return (QLocale::Palau);
	} else if (iso3166 == "ps") {
		return (QLocale::PalestinianTerritory);
	} else if (iso3166 == "pa") {
		return (QLocale::Panama);
	} else if (iso3166 == "pg") {
		return (QLocale::PapuaNewGuinea);
	} else if (iso3166 == "py") {
		return (QLocale::Paraguay);
	} else if (iso3166 == "pe") {
		return (QLocale::Peru);
	} else if (iso3166 == "ph") {
		return (QLocale::Philippines);
	} else if (iso3166 == "pn") {
		return (QLocale::Pitcairn);
	} else if (iso3166 == "pl") {
		return (QLocale::Poland);
	} else if (iso3166 == "pt") {
		return (QLocale::Portugal);
	} else if (iso3166 == "pr") {
		return (QLocale::PuertoRico);
	} else if (iso3166 == "qa") {
		return (QLocale::Qatar);
	} else if (iso3166 == "re") {
		return (QLocale::Reunion);
	} else if (iso3166 == "ro") {
		return (QLocale::Romania);
	} else if (iso3166 == "ru") {
		return (QLocale::RussianFederation);
	} else if (iso3166 == "rw") {
		return (QLocale::Rwanda);
	} else if (iso3166 == "kn") {
		return (QLocale::SaintKittsAndNevis);
	} else if (iso3166 == "lc") {
		return (QLocale::StLucia);
	} else if (iso3166 == "vc") {
		return (QLocale::StVincentAndTheGrenadines);
	} else if (iso3166 == "ws") {
		return (QLocale::Samoa);
	} else if (iso3166 == "sm") {
		return (QLocale::SanMarino);
	} else if (iso3166 == "st") {
		return (QLocale::SaoTomeAndPrincipe);
	} else if (iso3166 == "sa") {
		return (QLocale::SaudiArabia);
	} else if (iso3166 == "sn") {
		return (QLocale::Senegal);
	} else if (iso3166 == "cs") {
		return (QLocale::SerbiaAndMontenegro);
	} else if (iso3166 == "sc") {
		return (QLocale::Seychelles);
	} else if (iso3166 == "sl") {
		return (QLocale::SierraLeone);
	} else if (iso3166 == "sg") {
		return (QLocale::Singapore);
	} else if (iso3166 == "sk") {
		return (QLocale::Slovakia);
	} else if (iso3166 == "si") {
		return (QLocale::Slovenia);
	} else if (iso3166 == "sb") {
		return (QLocale::SolomonIslands);
	} else if (iso3166 == "so") {
		return (QLocale::Somalia);
	} else if (iso3166 == "za") {
		return (QLocale::SouthAfrica);
	} else if (iso3166 == "gs") {
		return (QLocale::SouthGeorgiaAndTheSouthSandwichIslands);
	} else if (iso3166 == "es") {
		return (QLocale::Spain);
	} else if (iso3166 == "lk") {
		return (QLocale::SriLanka);
	} else if (iso3166 == "sh") {
		return (QLocale::StHelena);
	} else if (iso3166 == "pm") {
		return (QLocale::StPierreAndMiquelon);
	} else if (iso3166 == "sd") {
		return (QLocale::Sudan);
	} else if (iso3166 == "sr") {
		return (QLocale::Suriname);
	} else if (iso3166 == "sj") {
		return (QLocale::SvalbardAndJanMayenIslands);
	} else if (iso3166 == "sz") {
		return (QLocale::Swaziland);
	} else if (iso3166 == "se") {
		return (QLocale::Sweden);
	} else if (iso3166 == "ch") {
		return (QLocale::Switzerland);
	} else if (iso3166 == "sy") {
		return (QLocale::SyrianArabRepublic);
	} else if (iso3166 == "tw") {
		return (QLocale::Taiwan);
	} else if (iso3166 == "tj") {
		return (QLocale::Tajikistan);
	} else if (iso3166 == "tz") {
		return (QLocale::Tanzania);
	} else if (iso3166 == "th") {
		return (QLocale::Thailand);
	} else if (iso3166 == "tg") {
		return (QLocale::Togo);
	} else if (iso3166 == "tk") {
		return (QLocale::Tokelau);
	} else if (iso3166 == "to") {
		return (QLocale::TongaCountry);
	} else if (iso3166 == "tt") {
		return (QLocale::TrinidadAndTobago);
	} else if (iso3166 == "tn") {
		return (QLocale::Tunisia);
	} else if (iso3166 == "tr") {
		return (QLocale::Turkey);
	} else if (iso3166 == "tm") {
		return (QLocale::Turkmenistan);
	} else if (iso3166 == "tc") {
		return (QLocale::TurksAndCaicosIslands);
	} else if (iso3166 == "tv") {
		return (QLocale::Tuvalu);
	} else if (iso3166 == "ug") {
		return (QLocale::Uganda);
	} else if (iso3166 == "ua") {
		return (QLocale::Ukraine);
	} else if (iso3166 == "ae") {
		return (QLocale::UnitedArabEmirates);
	} else if (iso3166 == "gb") {
		return (QLocale::UnitedKingdom);
	} else if (iso3166 == "us") {
		return (QLocale::UnitedStates);
	} else if (iso3166 == "um") {
		return (QLocale::UnitedStatesMinorOutlyingIslands);
	} else if (iso3166 == "uy") {
		return (QLocale::Uruguay);
	} else if (iso3166 == "uz") {
		return (QLocale::Uzbekistan);
	} else if (iso3166 == "vu") {
		return (QLocale::Vanuatu);
	} else if (iso3166 == "va") {
		return (QLocale::VaticanCityState);
	} else if (iso3166 == "ve") {
		return (QLocale::Venezuela);
	} else if (iso3166 == "vn") {
		return (QLocale::VietNam);
	} else if (iso3166 == "vg") {
		return (QLocale::BritishVirginIslands);
	} else if (iso3166 == "vi") {
		return (QLocale::USVirginIslands);
	} else if (iso3166 == "wf") {
		return (QLocale::WallisAndFutunaIslands);
	} else if (iso3166 == "eh") {
		return (QLocale::WesternSahara);
	} else if (iso3166 == "ye") {
		return (QLocale::Yemen);
	} else if (iso3166 == "yu") {
		return (QLocale::Yugoslavia);
	} else if (iso3166 == "zm") {
		return (QLocale::Zambia);
	} else if (iso3166 == "zw") {
		return (QLocale::Zimbabwe);
	} else if (iso3166 == "me") {
		return (QLocale::Montenegro);
	} else if (iso3166 == "rs") {
		return (QLocale::Serbia);
	} else if (iso3166 == "bl") {
		return (QLocale::SaintBarthelemy);
	} else if (iso3166 == "mf") {
		return (QLocale::SaintMartin);
	}
	return (QLocale::AnyCountry);
}

