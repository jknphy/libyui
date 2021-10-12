/*
  Copyright (c) 2000 - 2011 Novell, Inc.
  Copyright (c) 2018 - 2021 SUSE LLC

  This library is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) version 3.0 of the License. This library
  is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
  License for more details. You should have received a copy of the GNU
  Lesser General Public License along with this library; if not, write
  to the Free Software Foundation, Inc., 51 Franklin Street, Fifth
  Floor, Boston, MA 02110-1301 USA
*/

/*
  File:	      YQPkgHistoryDialog.cc
  Author:     Stanislav Visnovsky <visnov@suse.com>

  Textdomain "qt-pkg"
*/


#define YUILogComponent "qt-pkg"
#include <yui/YUILog.h>

#include <yui/qt/YQi18n.h>
#include <yui/qt/YQUI.h>
#include <yui/qt/utf8.h>
#include <yui/qt/YQSignalBlocker.h>

#include <zypp/parser/HistoryLogReader.h>
#include <zypp/Date.h>
#include <zypp/Edition.h>
#include <boost/ref.hpp>

#include <QApplication>
#include <QDesktopWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSplitter>
#include <QStyle>
#include <QList>
#include <QBoxLayout>
#include <QTreeWidget>
#include <QMessageBox>
#include <QEventLoop>

#include "YQPkgHistoryDialog.h"
#include "YQPkgList.h"
#include "YQIconPool.h"
#include "QY2LayoutUtils.h"


#define SPACING         4	// between subwidgets
#define MARGIN		9	// around the widget

#define FILENAME	"/var/log/zypp/history"

using std::endl;


/**
 * Helper class to format the zypp history actions into human-readable tree
 * widget items
 **/
struct HistoryItemCollector
{
private:

    QTreeWidget *     _datesTree;
    QTreeWidget *     _actionsTree;
    QTreeWidgetItem * _actionsDateItem;  // parent item for all actions of this date
    QString           _lastDate;         // initialized empty like all QStrings

public:

    HistoryItemCollector( QTreeWidget * datesTree,
                          QTreeWidget * actionsTree )
        : _datesTree( datesTree )
        , _actionsTree( actionsTree )
        {}


    bool operator() ( const zypp::HistoryLogData::Ptr & item_ptr )
    {
	QString actionDate = fromUTF8( item_ptr->date().form( "%e %B %Y" ) );

	if ( actionDate != _lastDate ) // First action for a new date?
	{
	    _lastDate = actionDate;

            // Create a new item for that date in the dates tree
            new QTreeWidgetItem( _datesTree, QStringList( actionDate ) );

            // Create a date item in the actions tree as a parent for the actions of that date
	    _actionsDateItem = new QTreeWidgetItem( _actionsTree, QStringList( actionDate ) );
	    _actionsDateItem->setExpanded( true );
	}

	QStringList columns;

	if ( item_ptr->action() == zypp::HistoryActionID::INSTALL_e )
	{
	    zypp::HistoryLogDataInstall * item = static_cast <zypp::HistoryLogDataInstall *>( item_ptr.get() );

	    columns << fromUTF8( item->name() );
	    columns << fromUTF8( item->edition().version() );
	}
        else if (  item_ptr->action() == zypp::HistoryActionID::REMOVE_e )
	{
	    zypp::HistoryLogDataRemove * item = static_cast <zypp::HistoryLogDataRemove *>( item_ptr.get() );

	    columns << fromUTF8( item->name() );
	    columns << fromUTF8( item->edition().version() );
	}
        else if (  item_ptr->action() == zypp::HistoryActionID::REPO_ADD_e )
	{
	    zypp::HistoryLogDataRepoAdd * item = static_cast <zypp::HistoryLogDataRepoAdd *>( item_ptr.get() );

	    columns << fromUTF8( item->alias() );
	    columns << fromUTF8( item->url().asString() );

	}
        else if (  item_ptr->action() == zypp::HistoryActionID::REPO_REMOVE_e )
	{
	    zypp::HistoryLogDataRepoRemove * item = static_cast <zypp::HistoryLogDataRepoRemove *>( item_ptr.get() );

	    columns << fromUTF8( item->alias() );

	}
        else if (  item_ptr->action() == zypp::HistoryActionID::REPO_CHANGE_ALIAS_e )
	{
	    zypp::HistoryLogDataRepoAliasChange * item = static_cast <zypp::HistoryLogDataRepoAliasChange *>( item_ptr.get() );

	    columns << fromUTF8( item->oldAlias() ) + " -> " + fromUTF8( item->newAlias() );
	}
        else if (  item_ptr->action() == zypp::HistoryActionID::REPO_CHANGE_URL_e )
	{
	    zypp::HistoryLogDataRepoUrlChange * item = static_cast <zypp::HistoryLogDataRepoUrlChange *>( item_ptr.get() );

	    columns << fromUTF8( item->alias() );
	    columns << fromUTF8( item->newUrl().asString() );
	}

        if ( ! columns.isEmpty() )
        {
            QTreeWidgetItem * actionItem = new QTreeWidgetItem( _actionsDateItem, columns );
            actionItem->setIcon( 0, actionIcon( item_ptr->action() ) );
        }

	return true;
    }


    QPixmap actionIcon( zypp::HistoryActionID id )
    {
        switch ( id.toEnum() )
        {
            case zypp::HistoryActionID::INSTALL_e:     return YQIconPool::pkgInstall();
            case zypp::HistoryActionID::REMOVE_e:      return YQIconPool::pkgDel();
            case zypp::HistoryActionID::REPO_REMOVE_e: return YQIconPool::treeMinus();
            case zypp::HistoryActionID::REPO_ADD_e:    return YQIconPool::treePlus();
            default: return QPixmap();
        }

        return QPixmap();
    }
};




YQPkgHistoryDialog::YQPkgHistoryDialog( QWidget * parent )
    : QDialog( parent )
{
    // Dialog title
    setWindowTitle( _( "Package History" ) );

    setSizeGripEnabled( true );
    setMinimumSize( 750, 550 );

    QVBoxLayout * layout = new QVBoxLayout();
    Q_CHECK_PTR( layout );
    setLayout( layout );
    layout->setMargin( MARGIN );
    layout->setSpacing( SPACING );


    QLabel * label = new QLabel(  _( "Show History (/var/log/zypp/history)" ), this );
    label->setFixedHeight( label->sizeHint().height() );
    layout->addWidget( label );

    // VBox for splitter
    QSplitter * splitter = new QSplitter( Qt::Horizontal, this );
    Q_CHECK_PTR( splitter );
    layout->addWidget( splitter );

    // History view
    _datesTree = new QTreeWidget( splitter);
    _datesTree->setColumnCount( 1 );
    _datesTree->setHeaderLabels( QStringList( _("Date") ) );

    _actionsTree = new QTreeWidget( splitter );
    _actionsTree->setColumnCount( 2 );
    _actionsTree->setHeaderLabels( QStringList( _("Action") ) << _("Version/URL") );
    _actionsTree->setColumnWidth( 0, 200 );

    splitter->setStretchFactor( 0, 1 );
    splitter->setStretchFactor( 1, 2 );

    // Button box (to center the single button)

    QHBoxLayout * hbox = new QHBoxLayout();
    Q_CHECK_PTR( hbox );
    hbox->setMargin(  MARGIN  );
    layout->addLayout( hbox );
    hbox->addStretch();

    QPushButton * closeButton = new QPushButton( _( "&Close" ), this );
    Q_CHECK_PTR( closeButton );
    hbox->addWidget( closeButton );
    closeButton->setDefault( true );

    connect( closeButton,       SIGNAL( clicked() ),
	     this,              SLOT  ( accept()  ) );

    connect( _datesTree,        SIGNAL( itemSelectionChanged () ),
	    this,               SLOT  ( selectDate ()           ) );

    connect( _actionsTree,      SIGNAL( itemSelectionChanged()  ),
	    this,               SLOT  ( selectAction ()         ) );
}


void
YQPkgHistoryDialog::showHistoryDialog( QWidget* parent)
{
    YQPkgHistoryDialog dialog( parent );

    // Give instant feedback: Show the dialog and process the "show" event
    // immediately so the window opens immediately (albeit empty)

    dialog.show();
    QEventLoop eventLoop;
    eventLoop.processEvents( QEventLoop::ExcludeUserInputEvents,
			     200 ); // millisec

    YQUI::ui()->busyCursor();
    dialog.populate(); // This takes a moment
    YQUI::ui()->normalCursor();

    dialog.exec();
}


void
YQPkgHistoryDialog::populate()
{
    HistoryItemCollector itemCollector( _datesTree, _actionsTree );
    zypp::parser::HistoryLogReader reader( FILENAME,
                                           zypp::parser::HistoryLogReader::Options(),
                                           boost::ref( itemCollector ) );

    try
    {
	reader.readAll();
    }
    catch (  const zypp::Exception & exception )
    {
        yuiWarning() << "CAUGHT zypp exception: " << exception.asUserHistory() << endl;
        showReadHistoryWarning( fromUTF8( exception.asUserHistory() ) );
    }
}


void
YQPkgHistoryDialog::showReadHistoryWarning( const QString & message )
{
    QMessageBox msgBox;

    // Translators: This is a (short) text indicating that something went
    // wrong while trying to read the history file.

    QString heading = _( "Unable to read history" );

    if (  heading.length() < 25 )    // Avoid very narrow message boxes
    {
        QString blanks;
        blanks.fill( ' ', 50 - heading.length() );
        heading += blanks;
    }

    msgBox.setText( heading );
    msgBox.setIcon( QMessageBox::Warning );
    msgBox.setInformativeText( message );
    msgBox.exec();
}


void
YQPkgHistoryDialog::selectDate()
{
    QString item = _datesTree->selectedItems().first()->text( 0 );
    QList<QTreeWidgetItem *> items = _actionsTree->findItems( item, Qt::MatchExactly, 0);

    if ( items.size() > 0 )
    {
        YQSignalBlocker( this );

	_actionsTree->expandItem( items.first() );
	_actionsTree->setCurrentItem( items.first() );
	_actionsTree->scrollToItem( items.first(), QAbstractItemView::PositionAtTop );
    }
}


void
YQPkgHistoryDialog::selectAction()
{
    QTreeWidgetItem * item = _actionsTree->selectedItems().first();

    // if this is not a top-level item, better pick a top-level one

    if ( item->parent() )
	item = item->parent();

    QList<QTreeWidgetItem *> items = _datesTree->findItems( item->text( 0 ),
                                                            Qt::MatchExactly | Qt::MatchRecursive,
                                                            0 );

    if (  items.size () > 0 )
    {
        YQSignalBlocker( this );
	_datesTree->setCurrentItem( items.first());
    }
}

