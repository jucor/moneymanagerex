/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 ********************************************************/

#include "categexp.h"
#include "budget.h"

#include "../htmlbuilder.h"
#include "../util.h"
#include "../mmOption.h"
#include "../mmgraphpie.h"

mmReportCategoryExpenses::mmReportCategoryExpenses
( mmCoreDB* core, mmDateRange* date_range, const wxString& title, int type)
: mmPrintableBase(core)
, date_range_(date_range)
, title_(title)
, type_(type)
, ignoreFutureDate_(mmIniOptions::instance().ignoreFutureTransactions_)
{}

wxString mmReportCategoryExpenses::getHTMLText()
{
    mmHTMLBuilder hb;
    hb.init();
    hb.addHeader(2, title_);

    bool with_date = date_range_->is_with_date();
    hb.DisplayDateHeading(date_range_->start_date(), date_range_->end_date(), with_date);

    hb.startCenter();

    // Add the graph
    mmGraphPie gg;
    hb.addImage(gg.getOutputFileName());

    hb.startTable("60%");
    hb.startTableRow();
    hb.addTableHeaderCell(_("Category"));
    hb.addTableHeaderCell(_("Amount"), true);
    hb.endTableRow();

    double grandtotal = 0.0;

    std::vector<ValuePair> valueList;
    std::map<int, std::map<int, std::map<int, double> > > categoryStats;
    core_->bTransactionList_.getCategoryStats(categoryStats
        , date_range_
        , ignoreFutureDate_
        , false
        , with_date);
    core_->currencyList_.LoadBaseCurrencySettings();

    for (const auto& category: core_->categoryList_.entries_)
    {
        int categs = 0;
        bool grandtotalseparator = true;
        double categtotal = 0.0;
        int categID = category->categID_;
        const wxString sCategName = category->categName_;
        double amt = categoryStats[categID][-1][0];
        if (type_ == GOES && amt < 0.0) amt = 0;
        if (type_ == COME && amt > 0.0) amt = 0;

        categtotal += amt;
        grandtotal += amt;

        if (amt != 0)
        {
            ValuePair vp;
            vp.label = sCategName;
            vp.amount = amt;
            valueList.push_back(vp);

            hb.startTableRow();
            hb.addTableCell(sCategName, false, true);
            hb.addMoneyCell(amt, false);
            hb.endTableRow();
        }

        for (const auto & sub_category: category->children_)
        {
            int subcategID = sub_category->categID_;

            wxString sFullCategName = core_->categoryList_.GetFullCategoryString(categID, subcategID);
            amt = categoryStats[categID][subcategID][0];

            if (type_ == GOES && amt < 0.0) amt = 0;
            if (type_ == COME && amt > 0.0) amt = 0;

            categtotal += amt;
            grandtotal += amt;

            if (amt != 0)
            {
                categs++;
                ValuePair vp;
                vp.label = sFullCategName;
                vp.amount = amt;
                valueList.push_back(vp);

                hb.addTableRow(sFullCategName, amt);
            }
        }

        if (categs>1)
        {
            hb.addRowSeparator(0);
            hb.startTableRow();
            hb.addTableCell(_("Category Total: "),false, true, true, "GRAY");
            hb.addMoneyCell(categtotal, "GRAY");
            hb.endTableRow();
        }

        if (categs>0)
        {
            grandtotalseparator = false;
            hb.addRowSeparator(2);
        }
    }

    hb.addTotalRow(_("Grand Total: "), 1, grandtotal);

    hb.endTable();
    hb.endCenter();
    hb.end();

    gg.init(valueList);
    gg.Generate(wxEmptyString);

    return hb.getHTMLText();
}

mmReportCategoryExpensesGoes::mmReportCategoryExpensesGoes
( mmCoreDB* core, mmDateRange* date_range, const wxString& title)
: mmReportCategoryExpenses(core, date_range, title, 2)
{}

mmReportCategoryExpensesGoesCurrentMonth::mmReportCategoryExpensesGoesCurrentMonth
( mmCoreDB* core)
: mmReportCategoryExpensesGoes(core
, new mmCurrentMonth()
, _("Where the Money Goes - Current Month"))
{}

mmReportCategoryExpensesGoesCurrentMonthToDate::mmReportCategoryExpensesGoesCurrentMonthToDate
( mmCoreDB* core)
: mmReportCategoryExpensesGoes(core
, new mmCurrentMonthToDate()
, _("Where the Money Goes - Current Month to Date"))
{}

mmReportCategoryExpensesGoesLastMonth::mmReportCategoryExpensesGoesLastMonth
( mmCoreDB* core)
: mmReportCategoryExpensesGoes(core
, new mmLastMonth()
, _("Where the Money Goes - Last Month"))
{}

mmReportCategoryExpensesGoesLast30Days::mmReportCategoryExpensesGoesLast30Days
( mmCoreDB* core)
: mmReportCategoryExpensesGoes(core
, new mmLast30Days()
, _("Where the Money Goes - Last 30 Days"))
{}

mmReportCategoryExpensesGoesLastYear::mmReportCategoryExpensesGoesLastYear
( mmCoreDB* core)
: mmReportCategoryExpensesGoes(core
, new mmLastYear()
,  _("Where the Money Goes - Last Year"))
{}

mmReportCategoryExpensesGoesCurrentYear::mmReportCategoryExpensesGoesCurrentYear(mmCoreDB* core)
: mmReportCategoryExpensesGoes(core
, new mmCurrentYear()
, _("Where the Money Goes - Current Year"))
{}

mmReportCategoryExpensesGoesCurrentYearToDate::mmReportCategoryExpensesGoesCurrentYearToDate
( mmCoreDB* core)
: mmReportCategoryExpensesGoes(core
, new mmCurrentYearToDate()
, _("Where the Money Goes - Current Year to Date"))
{}

mmReportCategoryExpensesGoesLastFinancialYear::mmReportCategoryExpensesGoesLastFinancialYear
( mmCoreDB* core, const int day, const int month)
: mmReportCategoryExpensesGoes(core
, new mmLastFinancialYear(day, month)
, _("Where the Money Goes - Last Financial Year"))
{}

mmReportCategoryExpensesGoesCurrentFinancialYear::mmReportCategoryExpensesGoesCurrentFinancialYear
( mmCoreDB* core, const int day, const int month)
: mmReportCategoryExpensesGoes(core
, new mmCurrentFinancialYear(day, month)
, _("Where the Money Goes - Current Financial Year"))
{}

mmReportCategoryExpensesComes::mmReportCategoryExpensesComes
(mmCoreDB* core, mmDateRange* date_range, const wxString& title)
: mmReportCategoryExpenses(core, date_range, title, 1)
{}

mmReportCategoryExpensesComesCurrentMonth::mmReportCategoryExpensesComesCurrentMonth
( mmCoreDB* core)
: mmReportCategoryExpensesComes(core
, new  mmCurrentMonth()
, _("Where the Money Comes From - Current Month"))
{}

mmReportCategoryExpensesComesCurrentMonthToDate::mmReportCategoryExpensesComesCurrentMonthToDate
( mmCoreDB* core)
: mmReportCategoryExpensesComes(core
, new mmCurrentMonthToDate()
, _("Where the Money Comes From - Current Month to Date"))
{}

mmReportCategoryExpensesComesLastMonth::mmReportCategoryExpensesComesLastMonth
( mmCoreDB* core)
: mmReportCategoryExpensesComes(core
, new mmLastMonth()
,  _("Where the Money Comes From - Last Month"))
{}

mmReportCategoryExpensesComesLast30Days::mmReportCategoryExpensesComesLast30Days
( mmCoreDB* core)
: mmReportCategoryExpensesComes(core
, new mmLast30Days()
, _("Where the Money Comes From - Last Month"))
{}

mmReportCategoryExpensesComesLastYear::mmReportCategoryExpensesComesLastYear
( mmCoreDB* core)
: mmReportCategoryExpensesComes(core
, new mmLastYear()
, _("Where the Money Comes From - Last Year"))
{}

mmReportCategoryExpensesComesCurrentYear::mmReportCategoryExpensesComesCurrentYear
( mmCoreDB* core)
: mmReportCategoryExpensesComes(core
, new mmCurrentYear()
, _("Where the Money Comes From - Current Year"))
{}

mmReportCategoryExpensesComesCurrentYearToDate::mmReportCategoryExpensesComesCurrentYearToDate
( mmCoreDB* core)
: mmReportCategoryExpensesComes(core
, new mmCurrentYearToDate()
, _("Where the Money Comes From - Current Year to Date"))
{}

mmReportCategoryExpensesComesLastFinancialYear::mmReportCategoryExpensesComesLastFinancialYear
( mmCoreDB* core, int day, int month)
: mmReportCategoryExpensesComes(core
, new mmLastFinancialYear(day, month)
, _("Where the Money Comes From - Last Financial Year"))
{}

mmReportCategoryExpensesComesCurrentFinancialYear::mmReportCategoryExpensesComesCurrentFinancialYear
( mmCoreDB* core, int day, int month)
: mmReportCategoryExpensesComes(core
, new mmCurrentFinancialYear(day, month)
, _("Where the Money Comes From - Current Financial Year"))
{}

mmReportCategoryExpensesCategories::mmReportCategoryExpensesCategories
( mmCoreDB* core, mmDateRange* date_range, const wxString& title)
: mmReportCategoryExpenses(core, date_range, title, 0)
{}

mmReportCategoryExpensesCategoriesCurrentMonth::mmReportCategoryExpensesCategoriesCurrentMonth
( mmCoreDB* core)
: mmReportCategoryExpensesCategories(core
, new mmCurrentMonth()
, _("Categories - Current Month"))
{}

mmReportCategoryExpensesCategoriesCurrentMonthToDate::mmReportCategoryExpensesCategoriesCurrentMonthToDate
( mmCoreDB* core)
: mmReportCategoryExpensesCategories(core
, new mmCurrentMonthToDate()
, _("Categories - Current Month to Date"))
{}

mmReportCategoryExpensesCategoriesLastMonth::mmReportCategoryExpensesCategoriesLastMonth(mmCoreDB* core)
: mmReportCategoryExpensesCategories(core
, new mmLastMonth()
, _("Categories - Last Month"))
{}

mmReportCategoryExpensesCategoriesLast30Days::mmReportCategoryExpensesCategoriesLast30Days
( mmCoreDB* core)
: mmReportCategoryExpensesCategories(core
, new mmLast30Days()
, _("Categories - Last 30 Days"))
{}

mmReportCategoryExpensesCategoriesLastYear::mmReportCategoryExpensesCategoriesLastYear(mmCoreDB* core)
: mmReportCategoryExpensesCategories(core
, new mmLastYear()
, _("Categories - Last Year"))
{}

mmReportCategoryExpensesCategoriesCurrentYear::mmReportCategoryExpensesCategoriesCurrentYear
( mmCoreDB* core)
: mmReportCategoryExpensesCategories(core
, new mmCurrentYear()
, _("Categories - Current Year"))
{}

mmReportCategoryExpensesCategoriesCurrentYearToDate::mmReportCategoryExpensesCategoriesCurrentYearToDate
( mmCoreDB* core)
: mmReportCategoryExpensesCategories(core
, new mmCurrentYearToDate()
, _("Categories - Current Year to Date"))
{}

mmReportCategoryExpensesCategoriesLastFinancialYear::mmReportCategoryExpensesCategoriesLastFinancialYear
( mmCoreDB* core, int day, int month)
: mmReportCategoryExpensesCategories(core
, new mmLastFinancialYear(day, month)
, _("Categories - Last Financial Year"))
{}

mmReportCategoryExpensesCategoriesCurrentFinancialYear::mmReportCategoryExpensesCategoriesCurrentFinancialYear
( mmCoreDB* core, int day, int month)
: mmReportCategoryExpensesCategories(core
, new mmCurrentFinancialYear(day, month)
, _("Categories - Last Financial Year"))
{}
